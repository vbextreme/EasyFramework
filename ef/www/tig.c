#include <ef/tig.h>
#include <ef/file.h>
#include <ef/err.h>
#include <git2.h>

void tig_begin(void){
	git_libgit2_init();
}

void tig_end(void){
	git_libgit2_shutdown();
}

__private int git_fetch_progress(const git_transfer_progress *stats, void *payload){
	tigProgress_s* p = payload;
	p->status = TIG_STATUS_FETCH;
	p->numTotal = stats->total_objects;
	p->num = stats->received_objects;
	if(p->progress) p->progress(p);
	return 0;
}

__private void git_checkout_progress(__unused const char *path, size_t cur, size_t tot, void *payload){
	tigProgress_s* p = payload;
	p->status = TIG_STATUS_CHECKOUT;
	p->num = cur;
	p->numTotal = tot;
	if( p->progress ) p->progress(p);
}

const char* tig_error_get(int* klass){
	const git_error *e = giterr_last();
	if( klass ) *klass = e->klass;
	return e->message;
}

__private void tig_pusherr(const char* info, int err){
	int klass;
	const char* strerr = tig_error_get(&klass);
	err_push("%s(%d|%d):%s", info, err, klass, strerr);
}

err_t tig_clone(const char* url, const char* path, tigProgress_f progress, void* userdata){
	dbg_info("url:%s", url);
	dbg_info("path:%s",path);

	int err = 0;
	tigProgress_s cp = {0,0,0,0,progress, userdata};
	
	git_repository *repo = NULL;
	git_clone_options repoopt; /* = GIT_CLONE_OPTIONS_INIT have bug */
	git_clone_init_options(&repoopt, GIT_CLONE_OPTIONS_VERSION);
	repoopt.checkout_opts.checkout_strategy = GIT_CHECKOUT_SAFE;
	repoopt.checkout_opts.progress_cb = git_checkout_progress;
	repoopt.checkout_opts.progress_payload = &cp;
	repoopt.fetch_opts.callbacks.transfer_progress = git_fetch_progress;
	repoopt.fetch_opts.callbacks.payload = &cp;

	err = git_clone(&repo, url, path, &repoopt);
	if( progress ){
		cp.status = TIG_STATUS_END;
		progress(&cp);
	}

	if( err != 0 ){
		tig_pusherr("git", err);
	}
	else if( repo ){
		git_repository_free(repo);
	}
	
	return err;
}

#define PARSE_COUNTING_OBJECTS "Counting objects:"
#define PARSE_COMPRESSING_OBJECTS "Compressing objects:"
__private int tig_pull_sideband_progress(__unused const char *str, __unused int len, __unused void *data){
	tigProgress_s* pp = data;

	char* parse;
	if( (parse=strstr(str, PARSE_COUNTING_OBJECTS)) ){
		pp->status = TIG_STATUS_OBJECTS;
		parse += strlen(PARSE_COUNTING_OBJECTS);
	}
	else if( (parse=strstr(str, PARSE_COMPRESSING_OBJECTS)) ){
		pp->status = TIG_STATUS_COMPRESS;
		parse += strlen(PARSE_COMPRESSING_OBJECTS);
	}
	else{
		//dbg_warning("parse \n%s\n/parse", str);
		return 0;
	}

	if( !(parse=strchr(parse, '(')) ){
		//dbg_warning("parse ( \n<%s>", str); 
		return 0;
	}
	++parse;

	char* en = NULL;
	pp->num = strtoul(parse, &en, 10);
	if( !en || *en != '/'){
		//dbg_warning("parse objects \n<%s>",parse); 
		return 0;
	}
	parse = en+1;
	
	pp->numTotal = strtoul(parse, &en, 10);
	if( !en || *en != ')'){
		//dbg_warning("parse objects \n<%s>",parse); 
		return 0;
	}
	if( pp->progress ) pp->progress(pp);

	return 0;
}

__private int tig_pull_update_progress(const char *refname, __unused const git_oid *a, __unused const git_oid *b, void *data){
	tigProgress_s* pp = data;
	//char a_str[GIT_OID_HEXSZ+1];
   	char b_str[GIT_OID_HEXSZ+1];

	git_oid_fmt(b_str, b);
	b_str[GIT_OID_HEXSZ] = 0;

	pp->str = refname;

	if( git_oid_iszero(a) ){
		pp->status = TIG_STATUS_NEW;
	}
	else{
	//	git_oid_fmt(a_str, a);
	//	a_str[GIT_OID_HEXSZ] = 0;
		pp->status = TIG_STATUS_UPDATE;
	}

	if( pp->progress ) pp->progress(pp);
	return 0;
}

__private int tig_pull_transfer_progress(const git_transfer_progress *stats, void *payload){
	tigProgress_s* pp = payload;

	if( stats->received_objects == stats->total_objects ){
		pp->status = TIG_STATUS_DELTA;
        pp->num = stats->indexed_deltas;
		pp->numTotal = stats->total_deltas;
		if( pp->progress ) pp->progress(pp);
	}
	else if( stats->total_objects > 0 ){
		pp->status = TIG_STATUS_FETCH;
		pp->num = stats->received_objects;
		pp->numTotal = stats->total_objects;
		if( pp->progress ) pp->progress(pp);
	}

	return 0;
}

err_t tig_pull(int* nothing, const char* path, const char* origin, tigProgress_f progress, void* userdata){
	int err;
	tigProgress_s pp = {0,0,0,0, progress, userdata};
	
	git_repository *repo = NULL;
	git_remote *remote = NULL;
	git_fetch_options fopt;// = GIT_FETCH_OPTIONS_INIT; have bug
	git_fetch_init_options(&fopt, GIT_FETCH_OPTIONS_VERSION);

	if( (err = git_repository_open(&repo, path)) || repo == NULL ){
		tig_pusherr("git repo open", err);
		goto ONERR;
	}
	
	if( (err = git_remote_lookup(&remote, repo, origin)) ){
		tig_pusherr("git remote loockup", err);
		goto ONERR;
	}
	
	fopt.callbacks.update_tips = tig_pull_update_progress;
	fopt.callbacks.sideband_progress = tig_pull_sideband_progress;
	fopt.callbacks.transfer_progress = tig_pull_transfer_progress;
	fopt.callbacks.payload = &pp;

	if( (err = git_remote_fetch(remote, NULL, &fopt, NULL)) ){
		tig_pusherr("git remote fetch", err);
		goto ONERR;
	}

ONERR:
	pp.status = TIG_STATUS_END;
	if( progress ) progress(&pp);
	if( nothing ) *nothing = ( pp.num == 0 && pp.numTotal == 0 ) ? 1 : 0;

	if( remote ) git_remote_free(remote);
	if( repo ) git_repository_free(repo);
	return err;
}








