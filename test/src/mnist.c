#include <ef/type.h>
#include <ef/file.h>
#include <ef/memory.h>
#include <ef/gnn.h>

#define MNIST_NN_INPUT 784
#define MNIST_NN_OUTPUT 10

#define MNIST_LEARNING_TEST 60000
#define MNIST_MAGIC_IMAGE 0x803
#define MNIST_MAGIC_LABEL 0x801

typedef struct mnist{
	char* name;
	FILE* fd;
	uint32_t magic;
	uint32_t count;
	uint32_t h;
	uint32_t w;
}mnist_s;

__private err_t mnist_open(mnist_s* db){
	db->fd = fopen(db->name, "r");
	if( !db->fd ){
		dbg_error("db image name %s", db->name);
		return -1;
	}
	return 0;
}

__private void mnist_close(mnist_s* db){
	if( db->fd ) fclose(db->fd);
	free(db->name);
}

__private err_t mnist_image_header(mnist_s* db){
	if( fread(&db->magic, sizeof(uint32_t), 1, db->fd) != 1){
		dbg_error("read magic");
		return -1;
	}
	db->magic = big_little(db->magic);
	if( db->magic != MNIST_MAGIC_IMAGE ){
		dbg_error("magic");
		return -1;
	}

	if( fread(&db->count, sizeof(uint32_t), 1, db->fd) != 1){
		dbg_error("read count");
		return -1;
	}
	db->count = big_little(db->count);

	if( fread(&db->h, sizeof(uint32_t), 1, db->fd) != 1){
		dbg_error("read row");
		return -1;
	}
	db->h = big_little(db->h);

	if( fread(&db->w, sizeof(uint32_t), 1, db->fd) != 1){
		dbg_error("read columns");
		return -1;
	}
	db->w = big_little(db->w);

	//dbg_info("db image:: img:%u w:%d h:%d", db->count, db->w, db->h);
	
	return 0;
}

__private err_t mnist_label_header(mnist_s* db){
	if( fread(&db->magic, sizeof(uint32_t), 1, db->fd) != 1){
		dbg_error("read magic");
		return -1;
	}
	db->magic = big_little(db->magic);
	if( db->magic != MNIST_MAGIC_LABEL ){
		dbg_error("magic");
		return -1;
	}

	if( fread(&db->count, sizeof(uint32_t), 1, db->fd) != 1){
		dbg_error("read count");
		return -1;
	}
	db->count = big_little(db->count);

	//dbg_info("db image:: lbl:%u", db->count);
	
	return 0;
}
/*
__private err_t mnist_read_data(g2dImage_s* img, char* label, double* data, mnist_s* dbi, mnist_s* dbl){
	if( fread(label, sizeof(char), 1, dbl->fd) != 1){
		dbg_error("read label");
		return -1;
	}

	g2d_init(img, dbi->w, dbi->h, X_COLOR_MODE);
	dbg_info("%u*%u",dbi->w, dbi->h);
	for( unsigned y = 0; y < dbi->h; ++y ){
		unsigned const row = g2d_row(img, y);
		g2dColor_t* line = (g2dColor_t*)g2d_pixel(img, row);
		for( unsigned x = 0; x < dbi->w; ++x, ++line, ++data){
			unsigned char gray;
			if( fread(&gray, sizeof(char), 1, dbi->fd) != 1){
				dbg_error("read color");
				return -1;
			}
			//dbg_info("read %u %u = %u", x, y, gray);
			gray = 255 - gray;
			*line = g2d_color_make(img, 255, gray, gray, gray);
			*data = gray/255.0;
			iassert(*data>=0.0 && *data <=1.0);
		}
	}

	return 0;
}
*/
__private err_t mnist_read_data(char* label, double* data, mnist_s* dbi, mnist_s* dbl){
	if( fread(label, sizeof(char), 1, dbl->fd) != 1){
		dbg_error("read label");
		return -1;
	}

	for( unsigned y = 0; y < dbi->h; ++y ){
		for( unsigned x = 0; x < dbi->w; ++x, ++data){
			unsigned char gray;
			if( fread(&gray, sizeof(char), 1, dbi->fd) != 1){
				dbg_error("read color");
				return -1;
			}
			gray = 255 - gray;
			*data = (((double)gray * 2.0)/255.0) - 1.0;
			iassert(*data>=-1.0 && *data <=1.0);
		}
	}

	return 0;
}

__private char mnist_nn_get_result(double const* out){
	double max = out[0];
	char l = 0;

	for( size_t i = 1; i < MNIST_NN_OUTPUT; ++i){
		if( max < out[i] ){
			max = out[i];
			l = i;
		}
	}
	return l;
}

__private char mnist_desired(double const* desired){
	for( size_t i = 0; i < MNIST_NN_OUTPUT; ++i){
		if( desired[i] > 0.0 ) return i;
	}
	iassert(desired == NULL);
	return 0;
}

int mnist_validate(double const* out, double const* desired){
	if( mnist_nn_get_result(out) == mnist_desired(desired) ){
		return 1;
	}
	return 0;
}

err_t mnist_train(nn_h nn, char* trainImage, char* trainLabel, double rate, double retry){
	mnist_s dbi = {0};
	mnist_s dbl = {0};
	dbi.name = path_resolve(trainImage);
	dbl.name = path_resolve(trainLabel);

	if( mnist_open(&dbi) || 
		mnist_open(&dbl) ||
		mnist_label_header(&dbl) ||
		mnist_image_header(&dbi)
	){
		mnist_close(&dbl);
		mnist_close(&dbi);
		return -1;
	}

	puts("read file");
	double** data = mem_many(double*, MNIST_LEARNING_TEST);
	double** desired = mem_many(double*, MNIST_LEARNING_TEST);

	for( size_t i = 0; i < MNIST_LEARNING_TEST; ++i ){
		data[i] = mem_many(double, dbi.w * dbi.h);
		desired[i] = mem_zero_many(double, MNIST_NN_OUTPUT);
		char label;
		if( mnist_read_data(&label, data[i], &dbi, &dbl) ){
			dbg_fail("to many data");
		}
		desired[i][(unsigned)label] = 1.0;
	}

	nn_training_repeat(nn, data, desired, MNIST_LEARNING_TEST, rate, retry, mnist_validate);

	mnist_close(&dbl);
	mnist_close(&dbi);
	
	for( size_t i = 0; i < MNIST_LEARNING_TEST; ++i ){
		free(data[i]);
		free(desired[i]);
	}
	free(data);
	free(desired);
	return 0;
}

err_t mnist_test(nn_h nn, char* testImage, char* testLabel, size_t ntest){
	mnist_s dbi = {0};
	mnist_s dbl = {0};
	
	dbi.name = path_resolve(testImage);
	dbl.name = path_resolve(testLabel);

	if( mnist_open(&dbi) || 
		mnist_open(&dbl) ||
		mnist_label_header(&dbl) ||
		mnist_image_header(&dbi)
	){
		mnist_close(&dbl);
		mnist_close(&dbi);
		return -1;
	}

	double* data = mem_many(double, dbi.w * dbi.h);
	char label;
	size_t test = 0;
	size_t fail = 0;
	
	while( ntest-->0 && !mnist_read_data(&label, data, &dbi, &dbl) ){
		double const* out = nn_run(nn, data);
		char retlbl = mnist_nn_get_result(out);
		if( retlbl != label ){
			++fail;
			//printf("nn:%d lbl:%d\n", retlbl, label);
		}
		++test;
	}
	
	printf("test:%lu win:%.2lf lose:%.2lf\n", test,(100.0*(double)(test-fail))/(double)test, (100.0*(double)fail)/(double)test);

	mnist_close(&dbl);
	mnist_close(&dbi);
	free(data);

	return 0;
}


