/*---------------------------------------------------------------------\
|								       |
|		       __   __	  ____ _____ ____		       |
|		       \ \ / /_ _/ ___|_   _|___ \		       |
|			\ V / _` \___ \ | |   __) |		       |
|			 | | (_| |___) || |  / __/		       |
|			 |_|\__,_|____/ |_| |_____|		       |
|								       |
|				core system			       |
|							 (C) SuSE GmbH |
\----------------------------------------------------------------------/

   File:	PkgModuleFunctionsPatch.cc

   Author:	Klaus Kaempf <kkaempf@suse.de>
   Maintainer:  Klaus Kaempf <kkaempf@suse.de>
   Summary:     Access to Patch Manager
   Namespace:   Pkg
   Purpose:	Access to PMPatchManager
		Handles YOU related Pkg::function (list_of_arguments) calls
		from WFMInterpreter.
/-*/


#include <ycp/y2log.h>
#include <PkgModule.h>
#include <PkgModuleFunctions.h>

#include <ycp/YCPVoid.h>
#include <ycp/YCPBoolean.h>
#include <ycp/YCPInteger.h>
#include <ycp/YCPSymbol.h>
#include <ycp/YCPString.h>
#include <ycp/YCPList.h>
#include <ycp/YCPMap.h>

using std::string;
using std::endl;



//-------------------------------------------------------------
/**   
   @builtin YouStatus

   @short get map with status information
   @return map

*/
YCPMap
PkgModuleFunctions::YouStatus ()
{
    YCPMap result;

    /* TODO FIXME
    result->add( YCPString( "error" ), YCPBoolean( false ) );
    
    InstYou &you = _y2pm.youPatchManager().instYou();
    PMYouProductPtr product = you.settings()->primaryProduct();

    if ( product )
    {
	result->add( YCPString( "product" ), YCPString( product->product() ) );
	result->add( YCPString( "version" ), YCPString( product->version() ) );
	result->add( YCPString( "basearch" ), YCPString( product->baseArch() ) );
	result->add( YCPString( "business" ), YCPBoolean( product->businessProduct() ) );

	result->add( YCPString( "mirrorurl" ), YCPString( product->youUrl() ) );
    }
    result->add( YCPString( "lastupdate" ), YCPInteger( you.lastUpdate() ) );
    result->add( YCPString( "installedpatches" ),
                 YCPInteger( you.installedPatches() ) );
    */
    return result;
}

/**
   @builtin YouSetServer

   @short Set server to be used for getting patches.
   @param map  you server map as returned from YouGetServers.
   @return string ""      success
   
           "args"  wrong arguments

           "error" other error
*/
YCPValue
PkgModuleFunctions::YouSetServer (const YCPMap& servers)
{
    /* TODO FIXME
    PMYouServer server = convertServerObject( servers );

    InstYou &you = _y2pm.youPatchManager().instYou();

    you.settings()->setPatchServer( server );
    */

    return YCPString( "" );
}


/**
   @builtin YouGetUserPassword

   @short Get username and password needed for access to server.
   @return map "username" success, "password" password
*/
YCPValue
PkgModuleFunctions::YouGetUserPassword ()
{
    /* TODO FIXME
    InstYou &you = _y2pm.youPatchManager().instYou();

    you.readUserPassword();
    */
    string username = "you.username()";
    string password = "you.password()";

    YCPMap result;
    result->add( YCPString( "username" ), YCPString( username ) );
    result->add( YCPString( "password" ), YCPString( password ) );

    return result;
}

/**
   @builtin YouSetUserPassword

   @short Set username and password needed for access to server.
   @param string  username
   @param string  password
   @param boolean save  true, if username/password should be saved to disk
   @return string success if empty, "args"  wrong arguments, "error" other error
*/
YCPValue
PkgModuleFunctions::YouSetUserPassword (const YCPString& user, const YCPString& passwd, const YCPBoolean& p)
{
    string username = user->value_cstr();
    string password = passwd->value_cstr();
    bool persistent = p->value();

    /* TODO FIXME
    _last_error =
        _y2pm.youPatchManager().instYou().setUserPassword( username,
                                                           password,
                                                           persistent );

    if ( _last_error ) {
        return YCPString( "error" );
    }
    */

    return YCPString("");
}

// ------------------------
/**
   @builtin YouGetServers

   @short get urls of patch servers

   @param list<map>  list of maps where results are stored. The maps have the following fields
                     set: "url" -> URL of server. "name" -> Descriptive name of server.  
                     "directory" -> Directory file used to get list of patches.
   @return string empty for success, "args"  bad args, "get"  error getting file from server
           "write" error writing file to disk and "read"  error reading file after download
*/
YCPString
PkgModuleFunctions::YouGetServers (YCPReference strings)
{
    /* TODO FIXME
    std::list<PMYouServer> servers;
    _last_error = _y2pm.youPatchManager().instYou().servers( servers );
    if ( _last_error ) {
      if ( _last_error == YouError::E_get_youservers_failed ) return YCPString( "get" );
      if ( _last_error == YouError::E_write_youservers_failed ) return YCPString( "write" );
      if ( _last_error == YouError::E_read_youservers_failed ) return YCPString( "read" );
      return YCPString( "Error getting you servers." );
    }

    YCPList result = strings->entry ()->value ()->asList ();
    std::list<PMYouServer>::const_iterator it;
    for( it = servers.begin(); it != servers.end(); ++it ) {
      YCPMap serverMap;
      serverMap->add( YCPString( "url" ), YCPString( (*it).url().asString() ) );
      serverMap->add( YCPString( "name" ), YCPString( (*it).name() ) );
      serverMap->add( YCPString( "directory" ), YCPString( (*it).directory() ) );
      serverMap->add( YCPString( "type" ), YCPString( (*it).typeAsString() ) );
      result->add( serverMap );
    }
    
    strings->entry ()->setValue (result);
*/

    return YCPString( "" );
}




/**
  @builtin YouGetDirectory

  @short retrieve directory file listing all available patches

  @return string empty string for success, "url" ->  url not valid, "login" -> login failed
          "error"-> other error
*/
YCPValue
PkgModuleFunctions::YouGetDirectory ()
{
    /* TODO FIXME
    InstYou &you = _y2pm.youPatchManager().instYou();

    _last_error = you.retrievePatchDirectory();
    if ( _last_error ) {
      if ( _last_error == MediaError::E_login_failed ) return YCPString( "login" );
      return YCPString( "error" );
    }
    */

    return YCPString( "" );
}

/**   
  @builtin YouRetrievePatchInfo

  @short retrieve patches
  @param boolean download_again   true if patches should be downloaded again
  @param boolean check_signatures  true if signatures should be checked.
  
  @return string empty string for success, "args" for  bad args, "media" for media error
          "sig" for  signature check failed, "abort" for user aborted operation, 
          "url" for  url not valid and "login" for login failed
*/
YCPValue
PkgModuleFunctions::YouRetrievePatchInfo (const YCPBoolean& download, const YCPBoolean& sig)
{
    bool reload = download->value();
    bool checkSig = sig->value();

    /* TODO FIXME
    InstYou &you = _y2pm.youPatchManager().instYou();

    you.settings()->setReloadPatches( reload );
    you.settings()->setCheckSignatures( checkSig );
    
    _last_error = you.retrievePatchInfo();
    if ( _last_error ) {
      if ( _last_error == MediaError::E_login_failed ) return YCPString( "login" );
      if ( _last_error.errClass() == PMError::C_MediaError ) return YCPString( "media" );
      if ( _last_error == YouError::E_bad_sig_file ) return YCPString( "sig" );
      if ( _last_error == YouError::E_user_abort ) return YCPString( "abort" );
      return YCPString( _last_error.errstr() );
    }
    */

    return YCPString( "" );
}

/**   
   @builtin YouProcessPatches
   @short Download and install patches.
   @return boolean

*/
YCPValue
PkgModuleFunctions::YouProcessPatches ()
{
    /* TODO FIXME
    _last_error = _y2pm.youPatchManager().instYou().processPatches();
    if ( _last_error ) return YCPError( _last_error.errstr(), YCPBoolean( false ) );
    */
    return YCPBoolean( true );
}

/**   
   @builtin YouSelectPatches

   @short select patches based on types.
   @return void

*/
YCPValue
PkgModuleFunctions::YouSelectPatches ()
{
    /* TODO FIXME
    int kinds = PMYouPatch::kind_security | PMYouPatch::kind_recommended |
                PMYouPatch::kind_patchlevel;

    _y2pm.youPatchManager().instYou().selectPatches( kinds );
*/    
    return YCPVoid ();
}

/* TODO FIXME
YCPMap
PkgModuleFunctions::YouPatch( const PMYouPatchPtr &patch )
{

    YCPMap result;

    result->add( YCPString( "kind" ), YCPString( patch->kindLabel( patch->kind() ) ) );
    result->add( YCPString( "name" ), YCPString( patch->name() ) );
    result->add( YCPString( "summary" ), YCPString( patch->shortDescription() ) );
    result->add( YCPString( "description" ), YCPString( patch->longDescription() ) );
    result->add( YCPString( "preinformation" ), YCPString( patch->preInformation() ) );
    result->add( YCPString( "postinformation" ), YCPString( patch->postInformation() ) );
    YCPList packageList;
    std::list<PMPackagePtr> packages = patch->packages();
    std::list<PMPackagePtr>::const_iterator itPkg;
    for ( itPkg = packages.begin(); itPkg != packages.end(); ++itPkg ) {
      packageList->add( YCPString( (*itPkg)->nameEd() ) );
    }
    
    result->add( YCPString( "packages" ), packageList );

    return result;
}
*/

/**
   @builtin YouRemovePackages

   @short remove downloaded packages.
   @return boolean
*/
YCPValue
PkgModuleFunctions::YouRemovePackages ()
{
    /* TODO FIXME
    _last_error = _y2pm.youPatchManager().instYou().removePackages();
    if ( _last_error ) return YCPError( _last_error.errstr(), YCPBoolean( false ) );
    */
    return YCPBoolean( true );
}