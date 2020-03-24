#include "test.h"
#include <ef/oauth.h>

/*
 https://www.oauth.com/playground/authorization-code.html
 authorization_code
 refresh_token
 implicit




 <BODY>{
  "access_token": "ya29.a0Adw1xeX93mhTnkpl1MhaCq9ZjjvDW7s1TIw32WmqMzBZTXryDScSU-M38MS4iySyLuIi522PqfUgSJWevIemajkDZDGF_qTV1wrwdzhBMh2ufJHesI1Re3PqMwHSZxH1amQV5KmmaDtDzknJ5CqH62T-CE9NCEuccoU",
  "expires_in": 3599,
  "refresh_token": "1//09NFH3TnYchCrCgYIARAAGAkSNwF-L9Ir_mKZSFgx4ELhyycPfXU_CVWcg57w81DRojW3o_aBef0gaQfQbMGKRA6JtcNPnkkaCIY",
  "scope": "https://www.googleapis.com/auth/calendar.readonly",
  "token_type": "Bearer"
}</BODY>

*/

#define AUTH_URL          "https://accounts.google.com/o/oauth2/v2/auth"
#define TOKEN_URL         "https://www.googleapis.com/oauth2/v4/token"
//#define TOKEN_URL         "https://accounts.google.com/o/oauth2/token"
#define REFRESH_TOKEN_URL "https://oauth2.googleapis.com/token"
//#define TOKEN_URL REFRESH_TOKEN_URL
#define CLIENT_ID         "981343212481-bc9ko35go4tj5pcvaasu99s4u67hcf97.apps.googleusercontent.com"
#define CLIENT_SECRET     "7hR9-gRKCj3_rEmJA-va45Ah"
#define AUTH_CODE         "4/xgF0TXX-ZFpZgZzEcrQ0V_jAQ87x9M0NyjVP-olouJwWVbIZjInNabQc16OBttrooH8RX1IyAxdsDLfOT2dkqMM"
#define SCOPE             "https://www.googleapis.com/auth/calendar.readonly"
#define REDIRECT_URI0     NULL
#define USERNAME          "hymer.vbx.com"
#define PASSWORD          NULL

/*@test -O --oauth 'test oauth'*/

/*@fn*/
void test_oauth(__unused const char* argA, __unused const char* argB){
	err_enable();

	oauth2Authorization_s auth = {
		.authorizationLink = AUTH_URL,
		.tokenLink = TOKEN_URL,
		.refreshTokenLink = REFRESH_TOKEN_URL,
		.httpReplyOk = NULL,
		.httpReplyError = NULL,
		.appClientId = CLIENT_ID,
		.appClientSecret = CLIENT_SECRET,
		.scope = SCOPE,
		.accessType = "offline"
	};

	oauth2_s* oa = oauth2_new(NULL);
	if( !oa ){
		err_print();
		return;
	}

	err_disable();
	oauth2_authorization_load(oa, &auth, "test.oauth");
	err_restore();

	if( !oauth2_authorization_ok(oa) ){
		dbg_info("oauth browser");
		if( oauth2_athorization_browser(oa, &auth, "chromium", 60000) ){
			dbg_error("shell");
			return;
		}

		dbg_info("oauth token get");
		if( oauth2_authorization_token_get(oa,&auth) ){
			err_print();
			oauth2_free(oa);
			return;
		}
	}

	long remaining = oauth2_authorization_remaining_time(oa);
	long gg  = ((remaining / 60) / 60) / 24;
	long ggs = gg * 60 * 60 * 24;
	long hh  = ((remaining - ggs) / 60) / 60;
	long hhs = hh * 60 * 60;
	long mm  = ((remaining - ggs) - hhs) / 60;
	long mms = mm * 60;
	long ss  = ((remaining - ggs) - hhs) - mms;
	
	dbg_info("remaining:: %ldg %ld:%ld:%ld", gg, hh, mm, ss);
	if( remaining < 60 ){
		return;
		if( oauth2_authorization_token_refresh(oa, &auth) ){
			err_print();
			return;
		}
	}

	if( oauth2_authorization_store(oa, &auth, "test.oauth") ){
		err_print();
		return;
	}

	oauth2_free(oa);
	err_restore();
}

