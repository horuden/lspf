/*
  Copyright (c) 2015 Daniel John Erdos

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

*/


enum TOKEN_TYPES
{
	TT_STRING_QUOTED,
	TT_STRING_UNQUOTED,
	TT_VARIABLE,
	TT_OTHER,
	TT_EOT
} ;


enum TOKEN_SUBTYPES
{
	TS_NAME,
	TS_AMPR_VAR_VALID,
	TS_AMPR_VAR_INVALID,
	TS_CTL_VAR_VALID,
	TS_CTL_VAR_INVALID,
	TS_COMPARISON_OP,
	TS_OPEN_BRACKET,
	TS_CLOSE_BRACKET,
	TS_COMMA,
	TS_EQUALS,
	TS_NONE
} ;


enum STATEMENT_TYPE
{
	ST_IF,
	ST_ELSE,
	ST_ERROR,
	ST_VGET,
	ST_VPUT,
	ST_ASSIGN,
	ST_GOTO,
	ST_EXIT,
	ST_REFRESH,
	ST_VERIFY,
	ST_EOF
} ;


enum IF_COND
{
	IF_EQ,
	IF_NE,
	IF_GT,
	IF_GE,
	IF_LE,
	IF_LT
} ;


enum VER_TYPE
{
	VER_NUMERIC,
	VER_LIST,
	VER_LISTX,
	VER_PICT,
	VER_HEX,
	VER_OCT,
	VER_NA
} ;


enum AS_FUNCTION
{
	AS_DIR,
	AS_EXISTS,
	AS_FILE,
	AS_LENGTH,
	AS_TRANS,
	AS_TRUNC,
	AS_REVERSE,
	AS_WORDS,
	AS_UPPER,
	AS_NONE
} ;


class token
{
	public:
		token()
		{
			value   = ""       ;
			idx     = -1       ;
			type    = TT_OTHER ;
			subtype = TS_NONE  ;
		}
		explicit token( TOKEN_TYPES tt )
		{
			value   = ""       ;
			type    = tt       ;
			subtype = TS_NONE  ;
		}
		void clear()
		{
			value   = ""       ;
			type    = TT_OTHER ;
			subtype = TS_NONE  ;
		}
		string value ;
		int    idx   ;
		TOKEN_TYPES    type    ;
		TOKEN_SUBTYPES subtype ;
} ;


class parser
{
	public:
		parser()
		{
			idx      = 0     ;
			optUpper = false ;
		}
		void   parseStatement( errblock&, const string& ) ;
		token  getFirstToken() ;
		token  getNextToken()  ;
		token  getToken( int ) ;
		int    getEntries()    ;
		void   eraseTokens( int ) ;
		void   optionUpper()   { optUpper = true ; }
		bool   getNextIfCurrent( TOKEN_TYPES    ) ;
		bool   getNextIfCurrent( TOKEN_SUBTYPES ) ;
		string peekNextValue()   ;
		token  getCurrentToken() ;
		string getCurrentValue() ;
		void   getNameList( errblock&, string& )  ;
		bool   isCurrentType( TOKEN_TYPES )       ;
		bool   isCurrentSubType( TOKEN_SUBTYPES ) ;
		void   getNextString( errblock& err, string::const_iterator&, const string& s, string& r, bool& ) ;
		STATEMENT_TYPE getStatementType() ;

	private:
		int   idx ;
		bool  optUpper ;
		set<string> ctl_valid = { { ".ALARM"   },
					  { ".AUTOSEL" },
					  { ".BROWSE"  },
					  { ".CSRPOS"  },
					  { ".CSRROW"  },
					  { ".CURSOR"  },
					  { ".EDIT"    },
					  { ".FALSE"   },
					  { ".HELP"    },
					  { ".MSG"     },
					  { ".NRET"    },
					  { ".PFKEY"   },
					  { ".RESP"    },
					  { ".TRUE"    },
					  { ".TRAIL"   },
					  { ".ZVARS"   } } ;
		token current_token ;
		vector<token>tokens ;
} ;

class pnts
{
	public:
		pnts() {} ;
		void parse( errblock&, string ) ;

		string pnts_field ;
		string pnts_var   ;
		string pnts_val   ;
} ;


class TRUNC
{
	public:
		TRUNC()
		{
			trnc_char = ' ' ;
			trnc_len  = 0   ;
		} ;

		void parse( errblock&, parser&, bool =true ) ;

		string trnc_field ;
		char   trnc_char  ;
		int    trnc_len   ;

} ;


class TRANS
{
	public:
		TRANS()
		{
			trns_msgid   = ""    ;
			trns_default = ""    ;
			trns_tbfield = false ;
			trns_trunc   = NULL  ;
		} ;
		~TRANS()
		{
			delete trns_trunc ;
		}
		void parse( errblock&, parser& ) ;

		TRUNC* trns_trunc   ;
		string trns_field   ;
		string trns_msgid   ;
		string trns_default ;
		bool   trns_tbfield ;
		vector<pair<string,string>> trns_list ;
} ;



class ASSGN
{
	public :
		ASSGN()
		{
			as_lhs      = ""    ;
			as_rhs      = ""    ;
			as_isvar    = false ;
			as_isattr   = false ;
			as_istb     = false ;
			as_trunc    = NULL  ;
			as_trans    = NULL  ;
			as_function = AS_NONE ;
		}
		~ASSGN()
		{
			delete as_trunc ;
			delete as_trans ;
		}

		void parse( errblock&, parser& ) ;

		string as_lhs   ;
		string as_rhs   ;
		TRUNC* as_trunc ;
		TRANS* as_trans ;
		AS_FUNCTION as_function ;
		bool   as_isvar  ;
		bool   as_isattr ;
		bool   as_istb   ;
} ;



class VERIFY
{
	public:
		VERIFY()
		{
			ver_var     = ""     ;
			ver_type    = VER_NA ;
			ver_msgid   = ""     ;
			ver_nblank  = false  ;
			ver_tbfield = false  ;
			ver_pnfield = false  ;
		} ;

		void parse( errblock&, parser&, bool =true ) ;

		string ver_var     ;
		string ver_msgid   ;
		VER_TYPE ver_type  ;
		bool   ver_nblank  ;
		bool   ver_tbfield ;
		bool   ver_pnfield ;
		vector<string> ver_vlist ;
} ;


class VPUTGET
{
	public:
		VPUTGET()
		{
			vpg_pool = ASIS ;
		} ;

		void parse( errblock&, parser& ) ;
		bool     vpg_vput ;
		string   vpg_vars ;
		poolType vpg_pool ;
} ;


class IFSTMNT
{
	public :
		IFSTMNT()
		{
			if_lhs    = ""    ;
			if_rhs.clear()    ;
			if_true   = false ;
			if_AND    = false ;
			if_else   = false ;
			if_verify = NULL  ;
			if_trunc  = NULL  ;
			if_next   = NULL  ;
		}
		~IFSTMNT()
		{
			delete if_verify ;
			delete if_trunc  ;
			delete if_next   ;
		}
		void parse( errblock&, parser& ) ;
		void parse_cond( errblock&, parser& ) ;

		string if_lhs      ;
		vector<string> if_rhs ;
		VERIFY*  if_verify ;
		TRUNC*   if_trunc  ;
		IFSTMNT* if_next   ;
		bool     if_true   ;
		bool     if_AND    ;
		bool     if_else   ;
		IF_COND  if_cond   ;
} ;


class panstmnt
{
	public :
		panstmnt()
		{
			ps_label   = ""    ;
			ps_rlist   = ""    ;
			ps_column  = 0     ;
			ps_exit    = false ;
			ps_goto    = false ;
			ps_refresh = false ;
			ps_assgn   = NULL  ;
			ps_ver     = NULL  ;
			ps_if      = NULL  ;
			ps_else    = NULL  ;
			ps_vputget = NULL  ;
		}
		~panstmnt()
		{
			delete ps_assgn   ;
			delete ps_ver     ;
			delete ps_if      ;
			delete ps_vputget ;
		}
		string   ps_label   ;
		string   ps_rlist   ;
		int      ps_column  ;
		bool     ps_exit    ;
		bool     ps_goto    ;
		bool     ps_refresh ;
		ASSGN*   ps_assgn   ;
		VERIFY*  ps_ver     ;
		IFSTMNT* ps_if      ;
		IFSTMNT* ps_else    ;
		VPUTGET* ps_vputget ;
} ;


class tbsearch
{
	public:
		tbsearch()
		{
			tbs_val   = ""    ;
			tbs_gen   = false ;
			tbs_cond  = s_EQ  ;
			tbs_scond = "EQ"  ;
			tbs_size  = 0     ;
		} ;

		explicit tbsearch( const string& val )
		{
			tbs_cond  = s_EQ  ;
			tbs_scond = "EQ"  ;
			if ( val.back() == '*' )
			{
				tbs_gen  = true ;
				tbs_size = val.size() - 1 ;
				tbs_val  = val.substr( 0, tbs_size ) ;
			}
			else
			{
				tbs_gen  = false ;
				tbs_size = 0     ;
				tbs_val  = val   ;
			}
		} ;

		bool setCondition( const string& cond )
		{
			if ( cond == "" )
			{
				tbs_scond = "GE" ;
				tbs_cond  = s_GE ;
				return true ;
			}
			tbs_scond = cond ;
			if      ( cond == "EQ" ) { tbs_cond = s_EQ ; }
			else if ( cond == "NE" ) { tbs_cond = s_NE ; }
			else if ( cond == "LE" ) { tbs_cond = s_LE ; }
			else if ( cond == "LT" ) { tbs_cond = s_LT ; }
			else if ( cond == "GE" ) { tbs_cond = s_GE ; }
			else if ( cond == "GT" ) { tbs_cond = s_GT ; }
			else                     { return false    ; }
			return true ;
		} ;

		string tbs_val   ;
		string tbs_scond ;
		srCOND tbs_cond  ;
		bool   tbs_gen   ;
		int    tbs_size  ;
} ;


class fieldExc
{
	public:
		fieldExc()
		{
			fieldExc_field   = "" ;
			fieldExc_command = "" ;
			fieldExc_passed  = "" ;
		} ;

		void parse( errblock&, string ) ;

		string fieldExc_field   ;
		string fieldExc_command ;
		string fieldExc_passed  ;

} ;


class panel_data
{
	public:
		explicit panel_data( int x )
		{
			screenId = x ;
		}

		int screenId ;
} ;


class slmsg
{
	public:
		slmsg()
		{
			smsg   = ""    ;
			lmsg   = ""    ;
			hlp    = ""    ;
			dvwin  = ""    ;
			dvtype = ""    ;
			dvalm  = ""    ;
			dvhlp  = ""    ;
			type   = IMT   ;
			alm    = false ;
			resp   = false ;
			smwin  = false ;
			lmwin  = false ;
		}
		void clear()
		{
			smsg   = ""    ;
			lmsg   = ""    ;
			hlp    = ""    ;
			dvwin  = ""    ;
			dvtype = ""    ;
			dvalm  = ""    ;
			dvhlp  = ""    ;
			type   = IMT   ;
			alm    = false ;
			resp   = false ;
			smwin  = false ;
			lmwin  = false ;
		}

		bool parse( const string&, const string& ) ;

		string  smsg   ;
		string  lmsg   ;
		string  hlp    ;
		string  dvwin  ;
		string  dvtype ;
		string  dvalm  ;
		string  dvhlp  ;
		cuaType type   ;
		bool    alm    ;
		bool    resp   ;
		bool    smwin  ;
		bool    lmwin  ;
} ;


class selobj
{
	public:
		selobj()
		{
			PGM     = "" ;
			PARM    = "" ;
			NEWAPPL = "" ;
			NEWPOOL = false ;
			PASSLIB = false ;
			SUSPEND = false ;
			SCRNAME = "" ;
			selPanl = false ;
			backgrd = false ;
		}
		void clear()
		{
			PGM     = "" ;
			PARM    = "" ;
			NEWAPPL = "" ;
			NEWPOOL = false ;
			PASSLIB = false ;
			SUSPEND = false ;
			SCRNAME = "" ;
			selPanl = false ;
			backgrd = false ;
		}
		bool parse( errblock&, string ) ;
		bool selPanel() { return selPanl ; }

		string PGM     ;
		string PARM    ;
		string NEWAPPL ;
		bool   NEWPOOL ;
		bool   PASSLIB ;
		bool   SUSPEND ;
		string SCRNAME ;
		bool   selPanl ;
		bool   backgrd ;

	private:
} ;


class logger
{
	public:
	logger()  ;
	~logger() ;

	ofstream& operator<<( const string& s )
	{
		of << s   ;
		return of ;
	}

	ofstream& operator<<( const ptime& t )
	{
		of << t   ;
		return of ;
	}

	void lock()    { mtx.lock()   ; }
	void unlock()  { mtx.unlock() ; }

	bool open( const string& ="", bool =false ) ;
	void close() ;
	bool set( const string& ) ;

	private:
	bool logOpen ;

	string* currfl ;
	string  tmpfl  ;
	string  logfl  ;

	ofstream of ;
	boost::mutex mtx ;
} ;
