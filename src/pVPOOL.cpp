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


using namespace boost::filesystem ;

// ******************************************************************************************************************************
// ************************************************** FUNCTION POOL SECTION *****************************************************
// ******************************************************************************************************************************


fPOOL::~fPOOL()
{
	// Free dynamic storage for all variables in the function pool when the pool is deleted

	map<string, stack<fVAR*> >::iterator it ;

	for ( it = POOL.begin() ; it!= POOL.end() ; it++ )
	{
		while ( !it->second.empty() )
		{
			delete it->second.top() ;
			it->second.pop() ;
		}
	}
}


void fPOOL::define( errblock& err,
		    const string& name,
		    string * addr,
		    nameCHCK check )
{
	err.setRC( 0 ) ;

	if ( check == CHECK && !isvalidName( name ) )
	{
		err.seterrid( "PSYE013A", "function pool DEFINE", name ) ;
		return ;
	}

	fVAR * var           = new fVAR ;
	var->fVAR_string_ptr = addr     ;
	var->fVAR_type       = STRING   ;
	var->fVAR_defined    = true     ;
	var->fVAR_system     = ( sysvar.count( name ) > 0 ) ;
	POOL[ name ].push( var )        ;
}


void fPOOL::define( errblock& err,
		    const string& name,
		    int * addr )
{
	err.setRC( 0 ) ;

	if ( !isvalidName( name ) )
	{
		err.seterrid( "PSYE013A", "function pool DEFINE", name ) ;
		return ;
	}

	fVAR * var        = new fVAR ;
	var->fVAR_int_ptr = addr     ;
	var->fVAR_type    = INTEGER  ;
	var->fVAR_defined = true     ;
	var->fVAR_system  = ( sysvar.count( name ) > 0 ) ;
	POOL[ name ].push( var )     ;
}


void fPOOL::dlete( errblock& err,
		   const string& name,
		   nameCHCK check )
{
	// Remove the vdefine for a variable from the function pool (delete dynamic storage for fVAR first)

	// Use * for all vdefined variables (except system variables that have been defined in the
	// pApplication constructor)

	// RC =  0 OK
	// RC =  8 Variable not found in the defined area of the function pool
	// RC = 20 Severe error

	map<string, stack<fVAR*> >::iterator it ;

	err.setRC( 0 ) ;

	if ( name == "*")
	{
		for ( it = POOL.begin() ; it != POOL.end() ; it++ )
		{
			while ( !it->second.top()->fVAR_system && it->second.top()->fVAR_defined )
			{
				while ( !it->second.empty() )
				{
					delete it->second.top() ;
					it->second.pop() ;
				}
				it = POOL.erase( it ) ;
				if ( it == POOL.end() ) { return ; }
			}
		}
		return ;
	}

	if ( check == CHECK && !isvalidName( name ) )
	{
		err.seterrid( "PSYE013A", "function pool DELETE", name ) ;
		return ;
	}

	it = POOL.find( name ) ;

	if ( it == POOL.end() || !it->second.top()->fVAR_defined )
	{
		err.setRC( 8 ) ;
		return ;
	}

	delete it->second.top() ;
	it->second.pop() ;

	if ( it->second.empty() ) { POOL.erase( it ) ; }
}


const string& fPOOL::get( errblock& err,
			  int maxRC,
			  const string& name,
			  nameCHCK check )
{
	map<string, stack<fVAR*> >::iterator it ;

	err.setRC( 0 ) ;

	if ( check == CHECK && !isvalidName( name ) )
	{
		err.seterrid( "PSYE013A", "function pool GET", name ) ;
		return nullstr ;
	}

	it = POOL.find( name ) ;
	if ( it == POOL.end() )
	{
		err.setRC( 8 ) ;
		if ( 8 > maxRC )
		{
			err.seterrid( "PSYE013B", d2ds( maxRC ), name ) ;
		}
		return nullstr ;
	}

	if ( it->second.top()->fVAR_type != STRING )
	{
		err.seterrid( "PSYE013C", name ) ;
		return nullstr ;
	}

	return *it->second.top()->fVAR_string_ptr ;
}


int fPOOL::get( errblock& err,
		int maxRC,
		dataType type,
		const string& name )
{
	map<string, stack<fVAR*> >::iterator it ;

	err.setRC( 0 ) ;

	if ( !isvalidName( name ) )
	{
		err.seterrid( "PSYE013A", "function pool GET", name ) ;
		return 0 ;
	}

	it = POOL.find( name ) ;
	if ( it == POOL.end() )
	{
		err.setRC( 8 ) ;
		if ( 8 > maxRC )
		{
			err.seterrid( "PSYE013B", d2ds( maxRC ), name ) ;
		}
		return 0 ;
	}

	if ( it->second.top()->fVAR_type != type )
	{
		err.seterrid( "PSYE013D", name ) ;
		return 0 ;
	}
	return *it->second.top()->fVAR_int_ptr ;
}


dataType fPOOL::getType( errblock& err,
			 const string& name,
			 nameCHCK check )
{
	map<string, stack<fVAR*> >::iterator it ;

	err.setRC( 0 ) ;

	if ( check == CHECK && !isvalidName( name ) )
	{
		err.seterrid( "PSYE013A", "function pool GETTYPE", name ) ;
		return ERROR ;
	}

	it = POOL.find( name ) ;
	if ( it == POOL.end() )
	{
		err.setRC( 8 ) ;
		return ERROR ;
	}

	return it->second.top()->fVAR_type ;
}


bool fPOOL::ifexists( errblock& err,
		      const string& name,
		      nameCHCK check )
{
	err.setRC( 0 ) ;

	if ( check == CHECK && !isvalidName( name ) )
	{
		err.seterrid( "PSYE013A", "function pool IFEXISTS", name ) ;
		return false ;
	}

	return POOL.find( name ) != POOL.end() ;
}


void fPOOL::put( errblock& err,
		 const string& name,
		 const string& value,
		 nameCHCK check )
{
	// RC =  0 OK
	// RC = 20 Severe error

	map<string, stack<fVAR*> >::iterator it ;

	err.setRC( 0 ) ;

	if ( check == CHECK && !isvalidName( name ) )
	{
		err.seterrid( "PSYE013A", "function pool PUT", name ) ;
		return ;
	}

	it = POOL.find( name ) ;
	if ( it == POOL.end() )
	{
		fVAR * var        = new fVAR ;
		var->fVAR_string  = value    ;
		var->fVAR_type    = STRING   ;
		var->fVAR_defined = false    ;
		var->fVAR_system  = ( sysvar.count( name ) > 0 ) ;
		POOL[ name ].push( var )     ;
		return ;
	}

	if ( it->second.top()->fVAR_type != STRING )
	{
		err.seterrid( "PSYE013C", name ) ;
		return ;
	}

	*(it->second.top()->fVAR_string_ptr) = value ;
}


void fPOOL::put( errblock& err,
		 const string& name,
		 int value )
{
	// RC =  0 OK
	// RC = 20 Severe error

	map<string, stack<fVAR*> >::iterator it ;

	err.setRC( 0 ) ;

	if ( !isvalidName( name ) )
	{
		err.seterrid( "PSYE013A", "function pool PUT", name ) ;
		return ;
	}

	it = POOL.find( name ) ;
	if ( it == POOL.end() )
	{
		fVAR * var        = new fVAR ;
		var->fVAR_int     = value    ;
		var->fVAR_type    = INTEGER  ;
		var->fVAR_defined = false    ;
		var->fVAR_system  = ( sysvar.count( name ) > 0 ) ;
		POOL[ name ].push( var )     ;
		return ;
	}

	if ( it->second.top()->fVAR_type != INTEGER )
	{
		err.seterrid( "PSYE013D", name ) ;
		return ;
	}

	*(it->second.top()->fVAR_int_ptr) = value ;
}


void fPOOL::reset( errblock& err )
{
	// Free dynamic storage for all variables in the function pool and clear the pool

	map<string, stack<fVAR*> >::iterator it ;

	err.setRC( 0 ) ;

	for ( it = POOL.begin() ; it!= POOL.end() ; it++ )
	{
		while ( !it->second.empty() )
		{
			delete it->second.top() ;
			it->second.pop() ;
		}
	}
	POOL.clear() ;
}


void fPOOL::setmask( errblock& err,
		     const string& name,
		     const string& mask )
{
	map<string, stack<fVAR*> >::iterator it ;

	err.setRC( 0 ) ;

	if ( !isvalidName( name ) )
	{
		err.seterrid( "PSYE013A", "function pool SETMASK", name ) ;
		return ;
	}

	it = POOL.find( name ) ;
	if ( it == POOL.end() )
	{
		err.setRC( 8 ) ;
		return ;
	}
	it->second.top()->fVAR_mask = mask ;
}


const string& fPOOL::vilist( int& RC,
			     vdType defn )
{
	map<string, stack<fVAR*> >::iterator it ;

	varList = "" ;
	varList.reserve( 9*POOL.size() ) ;

	RC = 8 ;
	for ( it = POOL.begin() ; it != POOL.end() ; it++ )
	{
		if ( !isvalidName( it->first ) ) { continue ; }
		if ( it->second.top()->fVAR_type != INTEGER ) { continue ; }
		if ( it->second.top()->fVAR_defined )
		{
			if ( defn == IMPLICIT ) { continue ; }
		}
		else
		{
			if ( defn == DEFINED  ) { continue ; }
		}
		varList += " " + it->first ;
		RC = 0 ;
	}
	return varList ;
}


const string& fPOOL::vslist( int& RC,
			     vdType defn )
{
	map<string, stack<fVAR*> >::iterator it ;

	varList = "" ;
	varList.reserve( 9*POOL.size() ) ;

	RC = 8 ;
	for ( it = POOL.begin() ; it != POOL.end() ; it++ )
	{
		if ( !isvalidName( it->first ) ) { continue ; }
		if ( it->second.top()->fVAR_type != STRING ) { continue ; }
		if ( it->second.top()->fVAR_defined )
		{
			if ( defn == IMPLICIT ) { continue ; }
		}
		else
		{
			if ( defn == DEFINED  ) { continue ; }
		}
		varList += " " + it->first ;
		RC = 0 ;
	}
	return varList ;
}


string * fPOOL::vlocate( errblock& err,
			 const string& name,
			 nameCHCK check )
{
	map<string, stack<fVAR*> >::iterator it ;

	err.setRC( 0 ) ;

	if ( check == CHECK && !isvalidName( name ) )
	{
		err.seterrid( "PSYE013A", "function pool VLOCATE", name ) ;
		return NULL ;
	}

	it = POOL.find( name ) ;

	if ( it == POOL.end() )
	{
		err.setRC( 8 ) ;
		return NULL ;
	}

	if ( it->second.top()->fVAR_type != STRING )
	{
		err.seterrid( "PSYE013C", name ) ;
		return NULL ;
	}

	return it->second.top()->fVAR_string_ptr ;
}


// *******************************************************************************************************************************
// *************************************************** VARIABLE POOL SECTION *****************************************************
// *******************************************************************************************************************************


pVPOOL::~pVPOOL()
{
	// Free dynamic storage for all variables in the pool when the pool is deleted

	map< string, pVAR*>::iterator it ;

	for ( it = POOL.begin() ; it != POOL.end() ; it++ )
	{
		delete it->second ;
	}
}


void pVPOOL::put( errblock& err,
		  const string& name,
		  const string& value,
		  vTYPE vtype )
{
	// RC =  0 Normal completion
	// RC = 12 Variable in read-only
	// RC = 16 Truncation occured
	// RC = 20 Severe error

	pVAR * var ;

	map<string, pVAR*>::iterator it ;

	err.setRC( 0 ) ;

	if ( !isvalidName( name ) )
	{
		err.seterrid( "PSYE013A", "variable pool PUT", name ) ;
		return ;
	}

	if ( readOnly )
	{
		err.seterrid( "PSYE015B", 12 ) ;
		return ;
	}

	if ( value.size() > 32767 )
	{
		err.seterrid( "PSYE015A", name, 16 ) ;
	}

	it = POOL.find( name ) ;
	if ( it == POOL.end() )
	{
		var              = new pVAR ;
		if ( value.size() > 32767 ) { var->pVAR_value.assign( value, 0, 32766 ) ; }
		else                        { var->pVAR_value = value                   ; }
		var->pVAR_system = ( vtype == SYSTEM ) ;
		var->pVAR_type   = pV_VALUE ;
		POOL[ name ]     = var ;
	}
	else
	{
		if ( it->second->pVAR_type != pV_VALUE )
		{
			err.seterrid( "PSYE015D", name ) ;
			return ;
		}
		if ( it->second->pVAR_system && vtype != SYSTEM )
		{
			err.seterrid( "PSYE014O", name ) ;
			return ;
		}
		if ( value.size() > 32767 ) { it->second->pVAR_value.assign( value, 0, 32766 ) ; }
		else                        { it->second->pVAR_value = value                   ; }
	}
	changed = true ;
}


void pVPOOL::put( errblock& err,
		  map<string, pVAR*>::iterator v_it,
		  const string& value,
		  vTYPE vtype )
{
	// This version of put uses the variable iterator and the variable will always exist (RC=0 from locateSubPool)

	// RC =  0 Normal completion
	// RC = 12 Variable in read-only
	// RC = 16 Truncation occured
	// RC = 20 Severe error

	err.setRC( 0 ) ;

	if ( readOnly )
	{
		err.seterrid( "PSYE015B", 12 ) ;
		return ;
	}

	if ( v_it->second->pVAR_type != pV_VALUE )
	{
		err.seterrid( "PSYE015D", v_it->first ) ;
		return ;
	}

	if ( v_it->second->pVAR_system && vtype != SYSTEM )
	{
		err.seterrid( "PSYE014O", v_it->first ) ;
		return ;
	}

	if ( value.size() > 32767 )
	{
		err.seterrid( "PSYE015A", v_it->first, 16 ) ;
		v_it->second->pVAR_value.assign( value, 0, 32766 ) ;
	}
	else
	{
		v_it->second->pVAR_value = value ;
	}
	changed = true ;
}


string pVPOOL::get( errblock& err,
		    map<string, pVAR*>::iterator v_it )
{
	// RC =  0 Normal completion
	// RC =  8 Variable not found (set the in pool manager)
	// RC = 20 Severe error

	// Generate the value for pV_type != pV_VALUE (these are date/time entries created on access)

	// TODO: Format date variables according to ZDATEF

	int    p1 ;
	string t  ;

	time_t rawtime        ;
	struct tm * time_info ;
	char   buf[ 12 ]      ;

	err.setRC( 0 ) ;

	std::stringstream stream;

	if ( v_it->second->pVAR_type != pV_VALUE &&
	     v_it->second->pVAR_type != pV_ZTIMEL  )
	{
		time( &rawtime ) ;
		time_info = localtime( &rawtime ) ;
	}

	switch( v_it->second->pVAR_type )
	{
		case pV_VALUE:    return v_it->second->pVAR_value ;
		case pV_ZTIME:    strftime( buf, sizeof(buf), "%H:%M", time_info ) ;
				  buf[ 5  ] = 0x00 ;
				  return buf       ;
		case pV_ZTIMEL:   stream << microsec_clock::local_time() ;
				  t  = stream.str()  ;
				  p1 = t.find( ' ' ) ;
				  return t.substr( p1+1, 11 ) ;
		case pV_ZDATE:    strftime( buf, sizeof(buf), "%d/%m/%y", time_info ) ;
				  buf[ 8  ] = 0x00 ;
				  return buf       ;
		case pV_ZDATEL:   strftime( buf, sizeof(buf), "%d/%m/%Y", time_info ) ;
				  buf[ 10 ] = 0x00 ;
				  return buf       ;
		case pV_ZDAY:     strftime( buf, sizeof(buf), "%d", time_info ) ;
				  buf[ 2  ] = 0x00 ;
				  return buf       ;
		case pV_ZDAYOFWK: strftime( buf, sizeof(buf), "%A", time_info ) ;
				  buf[ 9  ] = 0x00 ;
				  return buf      ;
		case pV_ZDATESTD: strftime( buf, sizeof(buf), "%Y/%m/%d", time_info ) ;
				  buf[ 10 ] = 0x00 ;
				  return buf       ;
		case pV_ZMONTH:   strftime( buf, sizeof(buf), "%m", time_info ) ;
				  buf[ 2  ] = 0x00 ;
				  return buf       ;
		case pV_ZJDATE:   strftime( buf, sizeof(buf), "%y.%j", time_info ) ;
				  buf[ 6  ] = 0x00 ;
				  return buf       ;
		case pV_ZJ4DATE:  strftime( buf, sizeof(buf), "%Y.%j", time_info ) ;
				  buf[ 8  ] = 0x00 ;
				  return buf       ;
		case pV_ZYEAR:    strftime( buf, sizeof(buf), "%y", time_info ) ;
				  buf[ 2  ] = 0x00 ;
				  return buf       ;
		case pV_ZSTDYEAR: strftime( buf, sizeof(buf), "%Y", time_info ) ;
				  buf[ 4  ] = 0x00 ;
				  return buf       ;
		default:          err.seterrid( "PSYE014P", v_it->first ) ;
				  return "" ;
	}
}


string * pVPOOL::vlocate( errblock& err,
			  map<string, pVAR*>::iterator v_it )
{
	// RC =  0 Normal completion
	// RC =  4 Variable generated on access.  NULL returned.
	// RC =  8 Variable not found (set in the pool manager).
	// RC = 20 Severe error (set in the pool manager).

	err.setRC( 0 ) ;

	if ( v_it->second->pVAR_type != pV_VALUE )
	{
		err.setRC( 4 ) ;
		return NULL ;
	}

	return &v_it->second->pVAR_value ;
}


void pVPOOL::erase( errblock& err,
		    map<string, pVAR*>::iterator v_it )
{
	// RC =  0 Normal completion
	// RC =  8 Variable not found (set in the pool manager)
	// RC = 12 Pool read-only or a system variable
	// RC = 16 Variable in the SYSTEM PROFILE pool (ISPS)
	// RC = 20 Severe error

	err.setRC( 0 ) ;

	if ( readOnly )
	{
		err.seterrid( "PSYE015B", 12 ) ;
		return ;
	}

	if ( v_it->second->pVAR_system )
	{
		err.seterrid( "PSYE015E", v_it->first, 12 ) ;
		return ;
	}

	if ( issysProfile() )
	{
		err.seterrid( "PSYE015C", 16 ) ;
		return ;
	}

	if ( v_it->second->pVAR_type != pV_VALUE )
	{
		err.seterrid( "PSYE015D", v_it->first ) ;
		return ;
	}

	delete v_it->second ;
	POOL.erase( v_it )  ;
	changed = true      ;
}


bool pVPOOL::isSystem( map<string, pVAR*>::iterator v_it )
{
	return v_it->second->pVAR_system ;
}


void pVPOOL::createGenEntries()
{
	pVAR * val ;
	pVAR * var ;

	val = new pVAR ;
	val->pVAR_value  = ""   ;
	val->pVAR_system = true ;

	var = new pVAR ; *var = *val ; var->pVAR_type = pV_ZTIME    ; POOL[ "ZTIME"    ] = var ;
	var = new pVAR ; *var = *val ; var->pVAR_type = pV_ZTIMEL   ; POOL[ "ZTIMEL"   ] = var ;
	var = new pVAR ; *var = *val ; var->pVAR_type = pV_ZDATE    ; POOL[ "ZDATE"    ] = var ;
	var = new pVAR ; *var = *val ; var->pVAR_type = pV_ZDATEL   ; POOL[ "ZDATEL"   ] = var ;
	var = new pVAR ; *var = *val ; var->pVAR_type = pV_ZDAY     ; POOL[ "ZDAY"     ] = var ;
	var = new pVAR ; *var = *val ; var->pVAR_type = pV_ZDAYOFWK ; POOL[ "ZDAYOFWK" ] = var ;
	var = new pVAR ; *var = *val ; var->pVAR_type = pV_ZDATESTD ; POOL[ "ZDATESTD" ] = var ;
	var = new pVAR ; *var = *val ; var->pVAR_type = pV_ZMONTH   ; POOL[ "ZMONTH"   ] = var ;
	var = new pVAR ; *var = *val ; var->pVAR_type = pV_ZJDATE   ; POOL[ "ZJDATE"   ] = var ;
	var = new pVAR ; *var = *val ; var->pVAR_type = pV_ZJ4DATE  ; POOL[ "ZJ4DATE"  ] = var ;
	var = new pVAR ; *var = *val ; var->pVAR_type = pV_ZYEAR    ; POOL[ "ZYEAR"    ] = var ;
	var = new pVAR ; *var = *val ; var->pVAR_type = pV_ZSTDYEAR ; POOL[ "ZSTDYEAR" ] = var ;

	delete val ;
}


void pVPOOL::load( errblock& err,
		   const string& applid,
		   const string& path )
{
	// RC = 0  Normal completion
	// RC = 20 Severe error

	string fname ;
	string hdr   ;
	string var   ;
	string value ;

	char x      ;
	int  i, k   ;
	int  n1, n2 ;
	char * buf1 ;
	char z[ 2 ] ;

	size_t buf1Size = 1024  ;

	fname = path + applid + "PROF" ;

	std::ifstream profile ;
	profile.open( fname.c_str() , ios::binary ) ;
	if ( !profile.is_open() )
	{
		err.seterrid( "PSYE015F", applid, fname ) ;
		return ;
	}

	buf1 = new char[ buf1Size ] ;

	profile.read (buf1, 2 ) ;
	if ( memcmp( buf1, "\x00\x84", 2 ) )
	{
		err.seterrid( "PSYE015G", applid, fname ) ;
		profile.close() ;
		delete[] buf1   ;
		return  ;
	}

	profile.get( x ) ;
	i = static_cast< int >( x ) ;
	if ( i > 1 )
	{
		err.seterrid( "PSYE015H", d2ds( i ), applid ) ;
		profile.close() ;
		delete[] buf1   ;
		return  ;
	}

	profile.get( x ) ;
	i = static_cast< int >( x ) ;
	if ( i < 0 ) { i = 256 + i ; }
	profile.read (buf1, i ) ;
	hdr.assign( buf1, i )   ;

	while ( true )
	{
		profile.get( x ) ;
		if ( profile.eof() ) { break ; }
		i = static_cast< int >( x ) ;
		if ( i < 0 ) { i = 256 + i ; }
		profile.read (buf1 , i) ;
		if ( profile.fail() != 0 ) { err.seterror() ; break ; }
		var.assign( buf1, i ) ;
		profile.read( z, 2 )  ;
		if ( profile.fail() != 0 ) { err.seterror() ; break ; }
		n1 = static_cast< int >( z[ 0 ] ) ;
		n2 = static_cast< int >( z[ 1 ] ) ;
		if ( n1 < 0 ) { n1 = 256 + n1 ; }
		if ( n2 < 0 ) { n2 = 256 + n2 ; }
		k = 256 * n1 + n2 ;
		if ( k > buf1Size )
		{
			delete[] buf1 ;
			buf1Size = k  ;
			buf1     = new char[ buf1Size ] ;
		}
		profile.read (buf1, k ) ;
		if ( profile.fail() != 0 ) { err.seterror() ; break ; }
		value.assign( buf1, k ) ;
		put( err, var, value ) ;
	}
	profile.close() ;
	delete[] buf1   ;
	if ( err.error() )
	{
		err.seterrid( "PSYE015I", applid ) ;
	}
	resetChanged() ;
}


void pVPOOL::save( errblock& err,
		   const string& applid )
{
	// RC = 0  Normal completion
	// RC = 4  Save not performed.  Pool in read-only or no changes made to pool
	// RC = 20 Severe error

	int i ;

	string fname ;

	err.setRC( 0 ) ;

	if ( readOnly || !changed ) { err.setRC( 4 ) ; return ; }

	if ( exists( path ) )
	{
		if ( !is_directory( path ) )
		{
			err.seterrid( "PSYE015J", path ) ;
			return ;
		}
	}
	else
	{
		err.seterrid( "PSYE015K", path ) ;
		return ;
	}
	if ( path.back() != '/' ) { path += "/" ; }
	fname = path + applid + "PROF" ;

	std::ofstream profile ;
	profile.open( fname.c_str(), ios::binary | ios::out ) ;
	profile << (char)00  ;  //
	profile << (char)132 ;  // x084 denotes a profile
	profile << (char)1   ;  // PROFILE format version 1
	profile << (char)44  ;  // Header length
	profile << "HDR                                         " ;

	map< string, pVAR*>::iterator it ;
	for ( it = POOL.begin() ; it != POOL.end() ; it++ )
	{
		i = it->first.size() ;
		profile << (char)i ;
		profile.write( it->first.c_str(), i ) ;
		i = it->second->pVAR_value.size() ;
		profile << (char)( i >> 8 ) ;
		profile << (char)( i ) ;
		profile.write( it->second->pVAR_value.c_str(), i ) ;
	}
	profile.close() ;
	resetChanged()  ;
}


// ******************************************************************************************************************************
// ************************************************** POOL MANAGER SECTION ******************************************************
// ******************************************************************************************************************************


poolMGR::poolMGR()
{
	// Create pools @DEFSHAR, @DEFPROF and @ROXPROF
	// @DEFPROF and @ROXPROF (Read Only Extention PROFILE) not currently used

	POOLs_shared[ "@DEFSHAR" ]  = new pVPOOL ;
	POOLs_shared[ "@DEFSHAR" ]->createGenEntries() ;

	POOLs_profile[ "@DEFPROF" ] = new pVPOOL ;
	POOLs_profile[ "@ROXPROF" ] = new pVPOOL ;

	shrdPool = 0 ;
}


poolMGR::~poolMGR()
{
	// Iterate over all remaining variable pools and delete to release dynamic storage

	for ( auto it = POOLs_shared.begin() ; it != POOLs_shared.end() ; it++ )
	{
		delete it->second ;
	}
	for ( auto it = POOLs_profile.begin() ; it != POOLs_profile.end() ; it++ )
	{
		delete it->second ;
	}
	for ( auto it = POOLs_lscreen.begin() ; it != POOLs_lscreen.end() ; it++ )
	{
		delete it->second ;
	}
}


void poolMGR::connect( int taskid, const string& ppool, int spool )
{
	// Add profile and shared pool names to the task table.  Increment use count for both.

	errblock err ;

	llog( "I", "Connecting task "<<taskid<<" to pool manager"<< endl ) ;
	llog( "I", "Profile pool: "<<ppool<< endl ) ;
	llog( "I", "Shared  pool: "<<d2ds( spool, 8 ) << endl ) ;

	boost::lock_guard<boost::mutex> lock( mtx ) ;

	task_table[ taskid ] = make_pair( ppool, spool ) ;

	POOLs_profile[ ppool ]->incRefCount() ;
	POOLs_shared[ d2ds( spool, 8 ) ]->incRefCount() ;
}


void poolMGR::disconnect( int taskid )
{
	// Remove task from the profile/shared task tables and decrement use counts.
	// If pool no longer in use, save if a profile, and delete it.

	errblock err ;

	string ppool ;
	string spool ;

	boost::lock_guard<boost::mutex> lock( mtx ) ;

	auto it = task_table.find( taskid ) ;
	ppool = it->second.first ;
	spool = d2ds( it->second.second, 8 ) ;

	auto itp = POOLs_profile.find( ppool ) ;
	auto its = POOLs_shared.find( spool )  ;
	itp->second->decRefCount() ;
	its->second->decRefCount() ;

	llog( "I", "Disconnecting task "<< taskid <<" from pool manager"<< endl ) ;
	task_table.erase( it ) ;

	if ( !itp->second->inUse() )
	{
		itp->second->save( err, itp->first ) ;
		if ( err.error() )
		{
			llog( "E", "Pool "<< itp->first <<" cannot be saved"<< endl ) ;
		}
		else
		{
			delete itp->second ;
			POOLs_profile.erase( itp ) ;
		}
	}

	if ( !its->second->inUse() )
	{
		delete its->second ;
		POOLs_shared.erase( its ) ;
	}
}


void poolMGR::setPools( errblock& err )
{
	// Lock is held when this is called, so don't lock (most routines)

	_shared = 0  ;
	_applid = "" ;

	if ( err.taskid == 0 )
	{
		llog( "E", "Logic error.  Task id cannot be zero "<< endl ) ;
		err.seterror() ;
		return ;
	}

	if ( task_table.count( err.taskid ) == 0 )
	{
		llog( "E", "Logic error.  Task "<< err.taskid <<" not connected to pool manager"<< endl ) ;
		err.seterror() ;
		return ;
	}

	_applid = task_table[ err.taskid ].first  ;
	_shared = task_table[ err.taskid ].second ;
}


void poolMGR::setPOOLsReadOnly()
{
	// Neither of these pools is currently used

	boost::lock_guard<boost::mutex> lock( mtx ) ;

	POOLs_profile[ "@ROXPROF" ]->setReadOnly() ;
	POOLs_profile[ "@DEFPROF" ]->setReadOnly() ;
}


void poolMGR::createProfilePool( errblock& err,
				 const string& ppool,
				 string path )
{
	// RC = 0  Pool created and loaded from existing file if a PROFILE pool
	// RC = 4  Pool created but not loaded as PROFILE file does not exist
	// RC = 20 Severe error

	string fname ;

	pVPOOL * pool  ;
	err.setRC( 0 ) ;

	boost::lock_guard<boost::mutex> lock( mtx ) ;

	if ( POOLs_profile.count( ppool ) > 0 ) { return ; }

	if ( path.back() != '/' ) { path += "/" ; }
	fname = path + ppool + "PROF" ;
	if ( exists( fname ) )
	{
		if ( !is_regular_file( fname ) )
		{
			llog( "E", "File "<< fname <<" is not a regular file for profile load"<< endl ) ;
			err.seterror() ;
		}
		else
		{
			pool       = new pVPOOL ;
			pool->path = path ;
			POOLs_profile[ ppool ] = pool ;
			llog( "I", "Pool " << ppool << " created okay.  Reading saved variables from profile dataset" << endl ) ;
			POOLs_profile[ ppool ]->load( err, ppool, path ) ;
			if ( ppool == "ISPS" )
			{
				POOLs_profile[ "ISPS" ]->sysProfile() ;
			}
		}
	}
	else if ( exists( path ) )
	{
		if ( !is_directory( path ) )
		{
			llog( "E", "Directory " << path << " is not a regular directory for profile load" <<  endl ) ;
			err.seterror() ;
		}
		else
		{
			llog( "I", "Profile "<< ppool+"PROF does not exist.  Creating default" <<endl ) ;
			pool       = new pVPOOL ;
			pool->path = path ;
			POOLs_profile[ ppool ] = pool ;
			llog( "I", "Profile Pool "<< ppool <<" created okay in path "<< path <<endl ) ;
			err.setRC( 4 ) ;
		}
	}
	else
	{
		llog( "E", "Directory "<< path <<" does not exist for profile load" <<  endl ) ;
		err.seterror() ;
	}
}


int poolMGR::createSharedPool()
{
	boost::lock_guard<boost::mutex> lock( mtx ) ;

	POOLs_shared[ d2ds( ++shrdPool, 8 ) ] = new pVPOOL ;

	return shrdPool ;
}


void poolMGR::createPool( int ls )
{
	// Create the logical-screen variable pool and add defaults.
	// This pool is not accessible by applications (internal use only).

	// Lock is held when this is called, so don't lock (put and get routines)

	errblock err  ;

	POOLs_lscreen[ ls ] = new pVPOOL ;
	POOLs_lscreen[ ls ]->put( err, "ZMSGID",   "",  USER ) ;
	POOLs_lscreen[ ls ]->put( err, "ZSCRNAME", "",  USER ) ;
	POOLs_lscreen[ ls ]->put( err, "ZSCRNAM2", "",  USER ) ;
	POOLs_lscreen[ ls ]->put( err, "ZSHMSGID", "N", USER ) ;
	POOLs_lscreen[ ls ]->put( err, "ZSHPANID", "N", USER ) ;
	POOLs_lscreen[ ls ]->put( err, "ZSHUSRID", "N", USER ) ;
}


void poolMGR::destroySystemPool( errblock& err )
{
	// Save ISPS pool and remove it from storage.  Called when lspf is terminating

	boost::lock_guard<boost::mutex> lock( mtx ) ;

	auto it = POOLs_profile.find( "ISPS" ) ;
	it->second->save( err, "ISPS" ) ;

	delete it->second ;
	POOLs_profile.erase( it ) ;
}


void poolMGR::destroyPool( int ls )
{
	// Remove the logical-screen pool when a logical screen is closed

	boost::lock_guard<boost::mutex> lock( mtx ) ;

	auto it = POOLs_lscreen.find( ls ) ;
	if ( it != POOLs_lscreen.end() )
	{
		delete it->second ;
		POOLs_lscreen.erase( it ) ;
	}
}


void poolMGR::statistics()
{
	string Mode ;

	errblock err ;

	boost::lock_guard<boost::mutex> lock( mtx ) ;

	llog( "-", "Pool Statistics:" << endl ) ;
	llog( "-", "         Number of shared pools . . . . " << POOLs_shared.size() << endl ) ;
	llog( "-", "         Number of profile pools. . . . " << POOLs_profile.size() << endl ) ;
	llog( "-", "         Number of connected tasks. . . " << task_table.size() << endl ) ;
	llog( "-", endl ) ;
	llog( "-", "         Shared pool details:" << endl ) ;

	for ( auto sp_it = POOLs_shared.begin() ; sp_it != POOLs_shared.end() ; sp_it++ )
	{
		Mode = sp_it->second->readOnly ? "RO" : "UP" ;
		llog( "-", "            Pool " << setw(8) << sp_it->first <<
			  "  use count: " << setw(4) << sp_it->second->refCount <<
			  "  " << Mode << "  entries: " << setw(5) << sp_it->second->POOL.size() << endl ) ;
	}
	llog( "-", endl ) ;
	llog( "-", "         Profile pool details:" << endl ) ;

	for ( auto pp_it = POOLs_profile.begin() ; pp_it != POOLs_profile.end() ; pp_it++ )
	{
		Mode = pp_it->second->readOnly ? Mode = "RO" : Mode = "UP" ;
		llog( "-", "            Pool " << setw(8) << pp_it->first << "  use count: " << setw(4) << pp_it->second->refCount <<
			  "  " << Mode << "  entries: " << setw(5) << pp_it->second->POOL.size() << "  path: " << pp_it->second->path << endl ) ;
	}

	llog( "-", endl ) ;
	llog( "-", "         Connected Tasks:" << endl ) ;
	for ( auto it = task_table.begin() ; it != task_table.end() ; it++ )
	{
		llog( "-", "            Id   " << setw(5) << it->first <<
		      " Profile: " << setw(4) << it->second.first <<
		      "  Shared: " << d2ds( it->second.second, 8 ) << endl) ;
	}
	llog( "-", "*************************************************************************************************************" << endl ) ;
}


void poolMGR::snap()
{
	errblock err ;

	string vtype ;

	map<string, pVPOOL*>::iterator sp_it ;
	map<string, pVPOOL*>::iterator pp_it ;
	map<string, pVAR*>::iterator v_it    ;

	boost::lock_guard<boost::mutex> lock( mtx ) ;

	llog( "-", "Pool Variables:" << endl ) ;
	llog( "-", "         Shared pool details:" << endl ) ;

	for ( sp_it = POOLs_shared.begin() ; sp_it != POOLs_shared.end() ; sp_it++ )
	{
		llog( "-", endl ) ;
		llog( "-", "         Pool " << setw(8) << sp_it->first << " use count:" << setw(3) << sp_it->second->refCount <<
			  " entries: " << sp_it->second->POOL.size() << endl ) ;
		for ( v_it = sp_it->second->POOL.begin() ; v_it != sp_it->second->POOL.end() ; v_it++ )
		{
			vtype = sp_it->second->isSystem( v_it ) ? vtype = " (SYS) " : vtype = " (USR) " ;
			llog( "-", setw(8) << std::left << v_it->first << vtype << sp_it->second->get( err, v_it ) << "<< " << endl ) ;
		}
	}
	llog( "-", endl ) ;
	llog( "-", "         Profile pool details:" << endl ) ;

	for ( pp_it = POOLs_profile.begin() ; pp_it != POOLs_profile.end() ; pp_it++ )
	{
		llog( "-", endl ) ;
		llog( "-", "         Pool " << setw(8) << pp_it->first << " use count: " << setw(3) << pp_it->second->refCount <<
			  " entries: " << pp_it->second->POOL.size() << endl ) ;
		llog( "-", "                            path: " << setw(3) << pp_it->second->path << endl ) ;
		for ( v_it = pp_it->second->POOL.begin() ; v_it != pp_it->second->POOL.end() ; v_it++ )
		{
			vtype = pp_it->second->isSystem( v_it ) ? vtype = " (SYS) " : vtype = " (USR) " ;
			llog( "-", setw(8) << std::left << v_it->first << vtype << pp_it->second->get( err, v_it ) << "<< " << endl ) ;
		}
	}
	llog( "-", "*************************************************************************************************************" << endl ) ;
}


const string& poolMGR::vlist( errblock& err,
			      int& RC,
			      poolType pType,
			      int lvl )
{
	map<string, pVPOOL*>::iterator p_it ;
	map<string, pVAR*>::iterator   v_it ;

	RC      = 0  ;
	varList = "" ;

	boost::lock_guard<boost::mutex> lock( mtx ) ;

	setPools( err ) ;
	if ( _applid == "" )
	{
		llog( "E", "Logic error.  _applid is null for vlist"<< endl ) ;
		return varList;
	}
	if ( _shared == 0 )
	{
		llog( "E", "Logic error.  _shared is null for vlist"<< endl ) ;
		return varList;
	}

	switch( pType )
	{
	case SHARED:
		switch ( lvl )
		{
		case 1:
			p_it = POOLs_shared.find( d2ds( _shared, 8 ) ) ;
			break ;
		case 2:
			p_it = POOLs_shared.find( "@DEFSHAR" ) ;
			break ;
		default:
			RC = 20 ;
			return varList ;
		}
		break ;
	case PROFILE:
		switch ( lvl )
		{
		case 1:
			p_it = POOLs_profile.find( _applid ) ;
			break ;
		case 2:
			p_it = POOLs_profile.find( "@ROXPROF" ) ;
			break ;
		case 3:
			p_it = POOLs_profile.find( "@DEFPROF" ) ;
			break ;
		case 4:
			p_it = POOLs_profile.find( "ISPS" ) ;
			break ;
		default:
			RC = 20 ;
			return varList ;
		}
		break ;

	default:
		RC = 20 ;
		return varList ;
	}

	varList.reserve( 9*p_it->second->POOL.size() ) ;
	for ( v_it = p_it->second->POOL.begin() ; v_it != p_it->second->POOL.end() ; v_it++ )
	{
		varList += " " + v_it->first ;
	}

	return varList ;
}


void poolMGR::sysput( errblock& err,
		      const string& name,
		      const string& value,
		      poolType pType )
{
	// Put variables to the profile ISPF pool or @DEFSHAR shared pool
	// These calls are not associated with an application

	boost::lock_guard<boost::mutex> lock( mtx ) ;

	switch ( pType )
	{
		case SHARED:
			POOLs_shared[ "@DEFSHAR" ]->put( err, name, value, SYSTEM ) ;
			break ;

		case PROFILE:
			POOLs_profile[ "ISPS" ]->put( err, name, value, SYSTEM ) ;
			break ;

		default:
			err.seterrid( "PSYE017D" ) ;
	}
}


void poolMGR::put( errblock& err,
		   const string& name,
		   const string& value,
		   poolType pType,
		   vTYPE vtype )
{
	// Pool search order:  ASIS, SHARED then PROFILE
	// RC = 0  variable put okay
	// RC = 12 variable not put as in read-only status
	// RC = 20 severe error

	// for put PROFILE, delete the variable from the SHARED pool

	map<string, pVPOOL*>::iterator sp_it ;
	map<string, pVPOOL*>::iterator pp_it ;
	map<string, pVPOOL*>::iterator p_it  ;

	map<string, pVAR*>::iterator v_it    ;

	errblock err2 ;

	if ( !isvalidName( name ) )
	{
		err.seterrid( "PSYE013A", "pool manager PUT", name ) ;
		return ;
	}

	boost::lock_guard<boost::mutex> lock( mtx ) ;

	setPools( err ) ;
	if ( _applid == "" )
	{
		llog( "E", "Logic error.  _applid is null for put "<<name<< endl ) ;
		return ;
	}
	if ( _shared == 0 )
	{
		llog( "E", "Logic error.  _shared is null for put "<<name<< endl ) ;
		return ;
	}

	sp_it = POOLs_shared.find( d2ds( _shared, 8 ) ) ;
	if ( sp_it == POOLs_shared.end() )
	{
		err.seterrid( "PSYE017E", _shared ) ;
		return   ;
	}

	switch( pType )
	{
	case ASIS:
		locateSubPool( err, p_it, v_it, _shared, name ) ;
		if      ( err.RC0() ) { p_it->second->put( err, v_it, value, vtype ) ; }
		else if ( err.RC8() )
		{
			locateSubPool( err, p_it, v_it, _applid, name ) ;
			if      ( err.RC0() ) {  p_it->second->put( err, v_it, value, vtype ) ; }
			else if ( err.RC8() ) { sp_it->second->put( err, name, value, vtype ) ; }
		}
		break ;

	case SHARED:
		locateSubPool( err, p_it, v_it, _shared, name ) ;
		if      ( err.RC0() ) {  p_it->second->put( err, v_it, value, vtype ) ; }
		else if ( err.RC8() ) { sp_it->second->put( err, name, value, vtype ) ; }
		break ;

	case PROFILE:
		pp_it = POOLs_profile.find( _applid ) ;
		locateSubPool( err, p_it, v_it, _applid, name ) ;
		if      ( err.RC0() ) {  p_it->second->put( err, v_it, value, vtype ) ; }
		else if ( err.RC8() ) { pp_it->second->put( err, name, value, vtype ) ; }
		locateSubPool( err2, p_it, v_it, _shared, name ) ;
		if ( err2.RC0() )     {  p_it->second->erase( err, v_it ) ; }
		break ;

	default:
		err.seterrid( "PSYE017C" ) ;
	}
}


void poolMGR::put( errblock& err,
		   int ls,
		   const string& name,
		   const string& value )
{
	// Set a variable from the logical-screen pool
	// Pool is created on first access

	boost::lock_guard<boost::mutex> lock( mtx ) ;

	err.setRC( 0 ) ;

	auto it = POOLs_lscreen.find( ls ) ;
	if ( it == POOLs_lscreen.end() )
	{
		createPool( ls ) ;
		it = POOLs_lscreen.find( ls ) ;
	}

	it->second->put( err, name, value, USER ) ;
}


string poolMGR::sysget( errblock& err,
			const string& name,
			poolType pType )
{
	// Get variables from the profile ISPF pool or @DEFSHAR shared pool
	// These calls are not associated with an application

	map<string, pVPOOL*>::iterator p_it ;
	map<string, pVAR*>::iterator v_it   ;

	boost::lock_guard<boost::mutex> lock( mtx ) ;

	switch ( pType )
	{
		case SHARED:
			p_it = POOLs_shared.find( "@DEFSHAR" ) ;
			break ;

		case PROFILE:
			p_it = POOLs_profile.find( "ISPS" ) ;
			break ;

		default:
			err.seterrid( "PSYE017D" ) ;
			return "" ;
	}

	v_it = p_it->second->POOL.find( name ) ;

	return p_it->second->get( err, v_it ) ;
}


string poolMGR::get( errblock& err,
		     const string& name,
		     poolType pType )
{
	// Pool search order: ASIS - SHARED then PROFILE
	// RC = 0  variable found
	// RC = 8  variable not found
	// RC = 20 severe error

	// for get PROFILE, delete the variable from the SHARED pool even if not found in the PROFILE pool

	errblock err2 ;

	map<string, pVPOOL*>::iterator p_it ;
	map<string, pVAR*>::iterator v_it   ;

	if ( !isvalidName( name ) )
	{
		err.seterrid( "PSYE013A", "pool manager GET", name ) ;
		return "" ;
	}

	boost::lock_guard<boost::mutex> lock( mtx ) ;

	setPools( err ) ;
	if ( _applid == "" )
	{
		llog( "E", "Logic error.  _applid is null for get "<<name<< endl ) ;
		return "";
	}
	if ( _shared == 0 )
	{
		llog( "E", "Logic error.  _shared is null for get "<<name<< endl ) ;
		return "";
	}

	switch ( pType )
	{
	case ASIS:
		locateSubPool( err, p_it, v_it, _shared, name ) ;
		if      ( err.RC0() ) { return p_it->second->get( err, v_it ) ; }
		else if ( err.RC8() )
		{
			locateSubPool( err, p_it, v_it, _applid, name ) ;
			if ( err.RC0() ) { return p_it->second->get( err, v_it ) ; }
		}
		break ;

	case PROFILE:
		locateSubPool( err2, p_it, v_it, _shared, name ) ;
		if ( err2.RC0() ) { p_it->second->erase( err, v_it ) ; }
		locateSubPool( err, p_it, v_it, _applid, name ) ;
		if ( err.RC0() ) { return p_it->second->get( err, v_it ) ; }
		break ;

	case SHARED:
		locateSubPool( err, p_it, v_it, _shared, name ) ;
		if ( err.RC0() ) { return p_it->second->get( err, v_it ) ; }
		break ;

	default:
		err.seterrid( "PSYE017C" ) ;
	}

	return "" ;
}


string poolMGR::get( errblock& err,
		     int ls,
		     const string& name )
{
	// Retrieve a variable from the logical-screen pool
	// Pool is created on first access

	err.setRC( 0 ) ;

	boost::lock_guard<boost::mutex> lock( mtx ) ;

	auto p_it = POOLs_lscreen.find( ls ) ;
	if ( p_it == POOLs_lscreen.end() )
	{
		createPool( ls ) ;
		p_it = POOLs_lscreen.find( ls ) ;
	}

	auto v_it = p_it->second->POOL.find( name ) ;
	if ( v_it == p_it->second->POOL.end() )
	{
		return "" ;
	}

	return p_it->second->get( err, v_it ) ;
}


string * poolMGR::vlocate( errblock& err,
			   const string& name,
			   poolType pType )
{
	// Pool search order: ASIS - SHARED then PROFILE
	// RC = 0  variable found
	// RC = 8  variable not found
	// RC = 20 severe error

	map<string, pVPOOL*>::iterator p_it ;
	map<string, pVAR*>::iterator v_it   ;

	err.setRC( 0 ) ;

	if ( !isvalidName( name ) )
	{
		err.seterrid( "PSYE013A", "pool manager VLOCATE", name ) ;
		return NULL ;
	}

	boost::lock_guard<boost::mutex> lock( mtx ) ;

	setPools( err ) ;
	if ( _applid == "" )
	{
		llog( "E", "Logic error.  _applid is null for vlocate "<<name<< endl ) ;
		return NULL ;
	}
	if ( _shared == 0 )
	{
		llog( "E", "Logic error.  _shared is null for vlocate "<<name<< endl ) ;
		return NULL;
	}

	switch ( pType )
	{
	case ASIS:
		locateSubPool( err, p_it, v_it, _shared, name ) ;
		if      ( err.RC0() ) { return p_it->second->vlocate( err, v_it ) ; }
		else if ( err.RC8() )
		{
			locateSubPool( err, p_it, v_it, _applid, name ) ;
			if ( err.RC0() ) { return p_it->second->vlocate( err, v_it ) ; }
		}
		break ;

	case PROFILE:
		locateSubPool( err, p_it, v_it, _applid, name ) ;
		if ( err.RC0() ) { return p_it->second->vlocate( err, v_it ) ; }
		break ;

	case SHARED:
		locateSubPool( err, p_it, v_it, _shared, name ) ;
		if ( err.RC0() ) { return p_it->second->vlocate( err, v_it ) ; }
		break ;

	default:
		err.seterrid( "PSYE017C" ) ;
	}

	return NULL ;
}



void poolMGR::erase( errblock& err,
		     const string& name,
		     poolType pType )
{
	// Pool search order: ASIS - SHARED then PROFILE
	// RC = 0  variable found and erased
	// RC = 8  variable not found
	// RC = 12 variable not erased as pool in read-only status, or system variable
	// RC = 16 variable not erased as in the ISPS SYSTEM profile pool
	// RC = 20 severe error

	map<string, pVPOOL*>::iterator p_it ;
	map<string, pVAR*>::iterator v_it   ;

	if ( !isvalidName( name ) )
	{
		err.seterrid( "PSYE013A", "pool manager ERASE", name ) ;
		return ;
	}

	boost::lock_guard<boost::mutex> lock( mtx ) ;

	setPools( err ) ;
	if ( _applid == "" )
	{
		llog( "E", "Logic error.  _applid is null for erase "<<name<< endl ) ;
		return ;
	}
	if ( _shared == 0 )
	{
		llog( "E", "Logic error.  _shared is null for erase "<<name<< endl ) ;
		return ;
	}

	switch( pType )
	{
	case ASIS:
		locateSubPool( err, p_it, v_it, _shared, name ) ;
		if ( err.RC0() ) { p_it->second->erase( err, v_it ) ; }
		else if ( err.RC8() )
		{
			locateSubPool( err, p_it, v_it, _applid, name ) ;
			if ( err.RC0() ) { p_it->second->erase( err, v_it ) ; }
		}
		break ;

	case PROFILE:
		locateSubPool( err, p_it, v_it, _applid, name ) ;
		if ( err.RC0() ) { p_it->second->erase( err, v_it ) ; }
		break ;

	case SHARED:
		locateSubPool( err, p_it, v_it, _shared, name ) ;
		if ( err.RC0() ) { p_it->second->erase( err, v_it ) ; }
		break ;

	case BOTH:
		locateSubPool( err, p_it, v_it, _shared, name ) ;
		if ( err.RC0() ) { p_it->second->erase( err, v_it ) ; }
		locateSubPool( err, p_it, v_it, _applid, name ) ;
		if ( err.RC0() ) { p_it->second->erase( err, v_it ) ; }
		break ;
	}
}


void poolMGR::locateSubPool( errblock& err,
			     map<string, pVPOOL*>::iterator& p_it,
			     map<string, pVAR*>::iterator& v_it,
			     const string& pool,
			     const string& name )
{
	// Locate the variable name in a profile sub-pool.
	// RC = 0 variable found.  Pool iterator p_it and variable iterator v_it, will be valid on return
	// RC = 8 if variable not found

	// Sub-Pool search order
	// PROFILE: APPLID, @ROXPROF @DEFPROF ISPS

	err.setRC( 0 ) ;

	p_it = POOLs_profile.find( pool ) ;
	v_it = p_it->second->POOL.find( name ) ;
	if ( v_it == p_it->second->POOL.end() )
	{
		p_it = POOLs_profile.find( "@ROXPROF" ) ;
		v_it = p_it->second->POOL.find( name ) ;
		if ( v_it == p_it->second->POOL.end() )
		{
			p_it = POOLs_profile.find( "@DEFPROF" ) ;
			v_it = p_it->second->POOL.find( name ) ;
			if ( v_it == p_it->second->POOL.end() )
			{
				p_it = POOLs_profile.find( "ISPS" ) ;
				v_it = p_it->second->POOL.find( name ) ;
				if ( v_it == p_it->second->POOL.end() ) { err.setRC( 8 ) ; }
			}
		}
	}
}


void poolMGR::locateSubPool( errblock& err,
			     map<string, pVPOOL*>::iterator& p_it,
			     map<string, pVAR*>::iterator& v_it,
			     int pool,
			     const string& name )
{
	// Locate the variable name in the shared pool sub-pool.
	// RC = 0 variable found.  Pool iterator p_it and variable iterator v_it, will be valid on return
	// RC = 8 if variable not found

	// Sub-Pool search order
	// SHARED:  CURRENT @DEFSHAR

	err.setRC( 0 ) ;

	p_it = POOLs_shared.find( d2ds( pool, 8 ) ) ;
	v_it = p_it->second->POOL.find( name ) ;
	if ( v_it == p_it->second->POOL.end() )
	{
		p_it = POOLs_shared.find( "@DEFSHAR" ) ;
		v_it = p_it->second->POOL.find( name )  ;
		if ( v_it == p_it->second->POOL.end() ) { err.setRC( 8 ) ; }
	}
}
