#ifndef EASYLIST_H_INCLUDED
#define EASYLIST_H_INCLUDED

#include <easytype.h>

///STD NAMESPACE head,tail,next,prev,hmap
///USE VAR FOR PELE AND NOT PASS &lst.head || &lst.tail 

typedef enum{LFIRST, LLAST, LAFTER, LBEFORE} LSTMODE;
#define LLEFT LAFTER
#define LRIGHT LBEFORE

#define lst_next(PELE) PELE = PELE->next
#define lst_prev(PELE) PELE = PELE->prev
#define lst_after(PELE) lst_prev(PELE)
#define lst_before(PELE) lst_before(PELE)

#define forlst(PLST,PELE) for(PELE = (PLST)->head; PELE; lst_next(PELE) ) 

#define forlstfree(PLST,PELE) while( (PELE = (PLST)->head) && ((PLST)->head = PELE->next) != PELE ) 

#define ele_init(PELE) PELE->next = PELE->prev = NULL										
#define lst_init(PLST) (PLST)->head = (PLST)->tail = NULL

#define lst_add(PLST,PNEW,MODE,PCUR) PPC_MULTILINE_START \
										if ( !((PLST)->head) )\
										{\
											(PLST)->head = (PLST)->tail = PNEW;\
											break;\
										}\
										\
										switch(MODE)\
										{\
											default:case LFIRST:\
												PNEW->next = (PLST)->head;\
												(PLST)->head->prev = PNEW;\
												(PLST)->head = PNEW;\
											break;\
											case LLAST:\
												PNEW->prev = (PLST)->tail;\
												(PLST)->tail->next = PNEW;\
												(PLST)->tail = PNEW;\
											break;\
											case LAFTER:\
												if ( !PCUR )\
												{\
													PNEW->next = (PLST)->head;\
													(PLST)->head->prev = PNEW;\
													(PLST)->head = PNEW;\
													break;\
												}\
												if ( PCUR->prev )\
												{\
													PNEW->prev = PCUR->prev;\
													PNEW->prev->next = PNEW;\
												}\
												PCUR->prev = PNEW;\
												PNEW->next = PCUR;\
												if ( (PLST)->head == PCUR ) (PLST)->head = PNEW;\
											break;\
											case LBEFORE:\
												if ( !PCUR ) \
												{\
													PNEW->prev = (PLST)->tail;\
													(PLST)->tail->next = PNEW;\
													(PLST)->tail = PNEW;\
												}\
												if ( PCUR->next )\
												{\
													PNEW->next = PCUR->next;\
													PNEW->next->prev = PNEW;\
												}\
												PCUR->next = PNEW;\
												PNEW->prev = PCUR;\
												if ( (PLST)->tail == PCUR ) (PLST)->tail = PNEW;\
											break;\
										}\
									PPC_MULTILINE_END
										
#define lst_remove(PLST,PELE)	PPC_MULTILINE_START \
									if ( (PLST)->head == PELE && (PLST)->tail == PELE  )\
									{\
										(PLST)->head = (PLST)->tail = NULL;\
										PELE->prev = NULL;\
										PELE->next = NULL;\
										break;\
									}\
									\
									if ( PELE->prev )\
										PELE->prev->next = PELE->next;\
									else \
										(PLST)->head = (PLST)->head->next;\
									\
									if ( PELE->next )\
										PELE->next->prev = PELE->prev;\
									else \
										(PLST)->tail = (PLST)->tail->prev;\
									\
									PELE->prev = NULL;\
									PELE->next = NULL;\
								PPC_MULTILINE_END

#define lst_debug_print(PELE) printf("<%7p>%7p<%7p>",PELE->prev,PELE,PELE->next)

#endif // EASYLIST_H_INCLUDED
