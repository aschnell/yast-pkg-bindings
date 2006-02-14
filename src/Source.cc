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

   File:	PkgModuleFunctionsSource.cc

   Author:	Klaus Kaempf <kkaempf@suse.de>
   Maintainer:  Klaus Kaempf <kkaempf@suse.de>

   Summary:     Access to Installation Sources
   Namespace:   Pkg
   Purpose:	Access to InstSrc
		Handles source related Pkg::function (list_of_arguments) calls
		from WFMInterpreter.
/-*/
//#include <unistd.h>
//#include <sys/statvfs.h>

#include <iostream>
#include <ycp/y2log.h>

#include <ycpTools.h>
#include <PkgModule.h>
#include <PkgModuleFunctions.h>

#include <zypp/SourceManager.h>
#include <zypp/SourceFactory.h>
#include <zypp/Source.h>
#include <zypp/Product.h>

using namespace std;

/****************************************************************************************
 * @builtin SourceSetRamCache
 * @short Allow/prevent InstSrces from caching package metadata on ramdisk
 * @description
 * In InstSys: Allow/prevent InstSrces from caching package metadata on ramdisk.
 * If no cache is used the media cannot be unmounted, i.e. no CD change possible.
 *
 * @param boolean allow  If true, allow caching
 * @return boolean
 **/
YCPValue
PkgModuleFunctions::SourceSetRamCache (const YCPBoolean& a)
{
    bool cache = a->value();
#warning SourceSetRamCache is not implemented
    return YCPBoolean( true );
}

/****************************************************************************************
 * @builtin SourceStartManager
 *
 * @short Make sure the InstSrcManager is up and knows all available InstSrces.
 * @description
 * Make sure the InstSrcManager is up and knows all available InstSrces.
 * Depending on the value of autoEnable, InstSources may be enabled on the
 * fly. It's safe to call this multiple times, and once the InstSources are
 * actually enabled, it's even cheap (enabling an InstSrc is expensive).
 *
 * @param boolean autoEnable If true, all InstSrces are enabled according to their default.
 * If false, InstSrces will be created in disabled state, and remain unchanged if
 * the InstSrcManager is already up.
 *
 * @return boolean
 **/
YCPValue
PkgModuleFunctions::SourceStartManager (const YCPBoolean& enable)
{
    try {
	zypp::SourceManager::sourceManager()->restore(_target_root);
	
	if( enable->value() )
	{
	    // go over all sources and get resolvables
	    std::list<unsigned int> ids = zypp::SourceManager::sourceManager()->enabledSources();
	    for( std::list<unsigned int>::iterator it = ids.begin(); it != ids.end(); ++it)
	    {
		zypp::Source_Ref src = zypp::SourceManager::sourceManager()->findSource(*it);
		zypp_ptr->addResolvables (src.resolvables());
	    }
	}
    }
    catch(...)
    {
	// FIXME: assuming the sources are already initialized
    }
    return YCPBoolean( true );
}

/****************************************************************************************
 * @builtin SourceStartCache
 *
 * @short Make sure the InstSrcManager is up, and return the list of SrcIds.
 * @description
 * Make sure the InstSrcManager is up, and return the list of SrcIds.
 * In fact nothing more than:
 *
 * <code>
 *   SourceStartManager( enabled_only );
 *   return SourceGetCurrent( enabled_only );
 * </code>
 *
 * @param boolean enabled_only If true, make sure all InstSrces are enabled according to
 * their default, and return the Ids of enabled InstSrces only. If false, return
 * the Ids of all known InstSrces.
 *
 * @return list<integer> list of SrcIds
 **/
YCPValue
PkgModuleFunctions::SourceStartCache (const YCPBoolean& enabled)
{
    SourceStartManager(enabled);
    
    return SourceGetCurrent(enabled);
}

/****************************************************************************************
 * @builtin SourceGetCurrent
 *
 * @short Return the list of all enabled InstSrc Ids.
 *
 * @param boolean enabled_only If true, or omitted, return the Ids of all enabled InstSrces.
 * If false, return the Ids of all known InstSrces.
 *
 * @return list<integer> list of SrcIds (integer)
 **/
YCPValue
PkgModuleFunctions::SourceGetCurrent (const YCPBoolean& enabled)
{
    std::list<unsigned int> ids = zypp::SourceManager::sourceManager()->enabledSources();
    
    YCPList res;
    
    for( std::list<unsigned int>::const_iterator it = ids.begin(); it != ids.end() ; ++it )
    {
	res->add( YCPInteger( *it ) );
    }

    return res;
}

/****************************************************************************************
 * @builtin SourceFinishAll
 *
 * @short Disable all InstSrces.
 * @return boolean
 **/
YCPValue
PkgModuleFunctions::SourceFinishAll ()
{
    try
    {
	y2milestone( "Storing the source setup in %s", _target_root.asString().c_str()) ;
	zypp::SourceManager::sourceManager()->store( _target_root );
	y2milestone( "Disabling all sources") ;
	zypp::SourceManager::sourceManager()->disableAllSources ();
    }
    catch (zypp::Exception & excpt)
    {
	y2error("Pkg::SourceFinishAll has failed: %s", excpt.msg().c_str() );
	return YCPBoolean(false);
    }

    y2milestone( "All sources finished");

    return YCPBoolean(true);
}

/////////////////////////////////////////////////////////////////////////////////////////
// Query individual sources
/////////////////////////////////////////////////////////////////////////////////////////

/****************************************************************************************
 * @builtin SourceGeneralData
 *
 * @short Get general data about the source
 * @description
 * Return general data about the source as a map:
 *
 * <code>
 * $[
 * "enabled"	: YCPBoolean,
 * "autorefresh": YCPBoolean,
 * "product_dir": YCPString,
 * "type"	: YCPString,
 * "url"	: YCPString
 * ];
 *
 * </code>
 * @param integer SrcId Specifies the InstSrc to query.
 * @return map
 **/
YCPValue
PkgModuleFunctions::SourceGeneralData (const YCPInteger& id)
{
    YCPMap data;
    zypp::Source_Ref src = zypp::SourceManager::sourceManager()->findSource(id->value());

   if ( ! src )
   {
	y2error ("Source %lld not found", id->value());
	return YCPVoid ();
   }

#warning SourceGeneralData doesn't return all data
    data->add( YCPString("enabled"),		YCPBoolean(src.enabled()));
    data->add( YCPString("autorefresh"),	YCPBoolean(src.autorefresh()));
    data->add( YCPString("type"),		YCPString(src.type()));
// FIXME:    data->add( YCPString("product_dir"),	YCPString("descr->product_dir().asString()"));

#warning SourceGeneralData returns URL without password
    // if password is required then use this parameter:
    // asString(url::ViewOptions() + url::ViewOptions::WITH_PASSWORD);
    data->add( YCPString("url"),		YCPString(src.url().asString()));
    return data;
}

/****************************************************************************************
 * @builtin SourceMediaData
 * @short Return media data about the source
 * @description
 * Return media data about the source as a map:
 *
 * <code>
 * $["media_count": YCPInteger,
 * "media_id"	  : YCPString,
 * "media_vendor" : YCPString,
 * "url"	  : YCPString,
 * ];
 * </code>
 *
 * @param integer SrcId Specifies the InstSrc to query.
 * @return map
 **/
YCPValue
PkgModuleFunctions::SourceMediaData (const YCPInteger& id)
{
    YCPMap data;

    zypp::Source_Ref src = zypp::SourceManager::sourceManager()->findSource(id->value());

    if (! src)
    {
	y2error ("Source %lld not found", id->value());
	return YCPVoid ();
    }

#warning SourceMediaData doesn't return all data
/*
  TODO FIXME:
  data->add( YCPString("media_count"),	YCPInteger((long long) 0));
  data->add( YCPString("media_id"),	YCPString("descr->media_id()"));
  data->add( YCPString("media_vendor"),	YCPString("descr->media_vendor()"));
*/
#warning SourceMediaData returns URL without password
  data->add( YCPString("url"),		YCPString(src.url().asString()));
  return data;
}

/****************************************************************************************
 * @builtin SourceProductData
 * @short Return Product data about the source
 * @param integer SrcId Specifies the InstSrc to query.
 * @description
 * Product data about the source as a map:
 *
 * <code>
 * $[
 * "productname"	: YCPString,
 * "productversion"	: YCPString,
 * "baseproductname"	: YCPString,
 * "baseproductversion"	: YCPString,
 * "vendor"		: YCPString,
 * "defaultbase"	: YCPString,
 * "architectures"	: YCPList(YCPString),
 * "requires"		: YCPString,
 * "linguas"		: YCPList(YCPString),
 * "label"		: YCPString,
 * "labelmap"		: YCPMap(YCPString lang,YCPString label),
 * "language"		: YCPString,
 * "timezone"		: YCPString,
 * "descrdir"		: YCPString,
 * "datadir"		: YCPString
 * ];
 * </code>
 *
 * @return map
 **/
YCPValue
PkgModuleFunctions::SourceProductData (const YCPInteger& id)
{
  zypp::Source_Ref src;

  try {
	src = zypp::SourceManager::sourceManager()->findSource(id->asInteger()->value());
  } catch(...) {
	y2error ("Source ID not found: %lld", id->asInteger()->value());
	return YCPVoid();
  }

#warning product category handling???

  zypp::Product::constPtr product;

  // find a product for the given source
  zypp::ResPool::byKind_iterator it = zypp_ptr->pool().byKindBegin(zypp::ResTraits<zypp::Product>::kind);

  for( ; it != zypp_ptr->pool().byKindEnd(zypp::ResTraits<zypp::Product>::kind)
        ; ++it) {
    product = boost::dynamic_pointer_cast<const zypp::Product>( it->resolvable() );

    y2debug ("Checking product: %s", product->displayName().c_str());
    if( product->source() == src )
    {
	y2debug ("Found");
 	break; 
    }
  }

  if( it == zypp_ptr->pool().byKindEnd(zypp::ResTraits<zypp::Product>::kind) )
  {
	y2error ("Product for source '%lld' not found", id->asInteger()->value());
	return YCPVoid();
  }

  YCPMap data;

  data->add( YCPString("label"),		YCPString( product->displayName() ) );
  data->add( YCPString("vendor"),		YCPString( product->vendor() ) );
  data->add( YCPString("productname"),		YCPString( product->name() ) );
  data->add( YCPString("productversion"),	YCPString( product->edition().version() ) );
  
#warning SourceProductData not finished 
/*  
  data->add( YCPString("baseproductname"),	YCPString( descr->content_baseproduct().asPkgNameEd().name ) );
  data->add( YCPString("baseproductversion"),	YCPString( descr->content_baseproduct().asPkgNameEd().edition.version() ) );
  data->add( YCPString("defaultbase"),		YCPString( descr->content_defaultbase() ) );

  InstSrcDescr::ArchMap::const_iterator it1 = descr->content_archmap().find ( _y2pm.baseArch() );
  if ( it1 != descr->content_archmap().end() ) {
    YCPList architectures;
    for ( std::list<PkgArch>::const_iterator it2 = it1->second.begin(); it2 != it1->second.end(); ++it2 ) {
      architectures->add ( YCPString( *it2 ) );
    }
    data->add( YCPString("architectures"),	architectures );
  }

  data->add( YCPString("requires"),		YCPString( descr->content_requires().asString() ) );

  YCPList linguas;
  for ( InstSrcDescr::LinguasList::const_iterator it = descr->content_linguas().begin();
	it != descr->content_linguas().end(); ++it ) {
    linguas->add( YCPString( it->code() ) );
  }
  data->add( YCPString("linguas"),		linguas );

 YCPMap labelmap;
  for ( InstSrcDescr::LabelMap::const_iterator it = descr->content_labelmap().begin();
	it != descr->content_labelmap().end(); ++it ) {
    labelmap->add( YCPString( it->first.code() ), YCPString( it->second ) );
  }
  data->add( YCPString("labelmap"),		labelmap );

  data->add( YCPString("language"),		YCPString( descr->content_language().code() ) );
  data->add( YCPString("timezone"),		YCPString( descr->content_timezone() ) );
  data->add( YCPString("descrdir"),		YCPString( descr->content_descrdir().asString() ) );
  data->add( YCPString("datadir"),		YCPString( descr->content_datadir().asString() ) );
*/
  return data;
}

/****************************************************************************************
 * @builtin SourceProduct
 * @short Get Product info
 * @param integer SrcId Specifies the InstSrc to query.
 *
 * <code>
 * $[
 *   "baseproduct":"",
 *   "baseversion":"", 
 *   "defaultbase":"i386", 
 *   "distproduct":"SuSE-Linux-DVD", 
 *   "distversion":"10.0", 
 *   "flags":"update", 
 *   "name":"SUSE LINUX", 
 *   "product":"SUSE LINUX 10.0", 
 *   "relnotesurl":"http://www.suse.com/relnotes/i386/SUSE-LINUX/10.0/release-notes.rpm",
 *   "requires":"suse-release-10.0",
 *   "vendor":"SUSE LINUX Products GmbH, Nuernberg, Germany", 
 *   "version":"10.0"
 * ] 
 * </code>
 * @return map Product info as a map
 **/
YCPValue
PkgModuleFunctions::SourceProduct (const YCPInteger& id)
{
    /* TODO FIXME */
  return YCPMap();
}

/****************************************************************************************
 * @builtin SourceProvideFile
 *
 * @short Make a file available at the local filesystem
 * @description
 * Let an InstSrc provide some file (make it available at the local filesystem).
 *
 * @param integer SrcId	Specifies the InstSrc .
 * @param integer medianr Number of the media the file is located on ('1' for the 1st media).
 * @param string file Filename relative to the media root.
 *
 * @return string local path as string
 **/
YCPValue
PkgModuleFunctions::SourceProvideFile (const YCPInteger& id, const YCPInteger& mid, const YCPString& f)
{
    zypp::Source_Ref src;

    try {
	src = zypp::SourceManager::sourceManager()->findSource(id->asInteger()->value());
    } catch(...) {
	y2error ("Source ID not found: %lld", id->asInteger()->value());
	return YCPVoid();
    }

    zypp::filesystem::Pathname path;
  
    try {
    	path = src.provideFile(f->value(), mid->asInteger()->value());
    } catch(...) {

#warning report proper error

	y2milestone ("File not found: %s", f->value_cstr());
	return YCPVoid();
    }

    return YCPString(path.asString());
}

/****************************************************************************************
 * @builtin SourceProvideDir
 * @short make a directory available at the local filesystem
 * @description
 * Let an InstSrc provide some directory (make it available at the local filesystem) and
 * all the files within it (non recursive).
 *
 * @param integer SrcId	Specifies the InstSrc .
 * @param integer medianr Number of the media the file is located on ('1' for the 1st media).
 * @param string dir Directoryname relative to the media root.
 * @return string local path as string
 */
YCPValue
PkgModuleFunctions::SourceProvideDir (const YCPInteger& id, const YCPInteger& mid, const YCPString& d)
{
    zypp::Source_Ref src = zypp::SourceManager::sourceManager()->findSource(id->asInteger()->value());
    zypp::filesystem::Pathname path;

    try
    {
	path = src.provideDir(d->value(), mid->asInteger()->value());
    }
    catch(...)
    {
	y2milestone ("Directory not found: %s", d->value_cstr());
	return YCPVoid();
    }

    return YCPString(path.asString());
/*
  YCPList args;
  args->add (id);
  args->add (mid);
  args->add (d);

  //-------------------------------------------------------------------------------------//
  YcpArgLoad decl(__FUNCTION__);

  InstSrcManager::ISrcId & source_id( decl.arg<YT_INTEGER, InstSrcManager::ISrcId>() );
  int &                    medianr  ( decl.arg<YT_INTEGER, int>() );
  Pathname &               dir     ( decl.arg<YT_STRING,  Pathname>() );

  if ( ! decl.load( args ) ) {
    return pkgError_bad_args;
  }
  //-------------------------------------------------------------------------------------//

  if ( ! source_id )
    return pkgError( InstSrcError::E_bad_id );

  Pathname localpath;
  PMError err = source_id->provideDir( medianr, dir, localpath );

  if ( err )
    return pkgError( err );

  return YCPString( localpath.asString() );
  */
}

/****************************************************************************************
 * @builtin SourceChangeUrl
 * @short Change Source URL
 * @description
 * Change url of an InstSrc. Used primarely when re-starting during installation
 * and a cd-device changed from hdX to srX since ide-scsi was activated.
 * @param integer SrcId Specifies the InstSrc.
 * @param string url The new url to use.
 * @return boolean
 **/
YCPValue
PkgModuleFunctions::SourceChangeUrl (const YCPInteger& id, const YCPString& u)
{
    /* TODO FIXME
  YCPList args;
  args->add (id);
  args->add (u);

  //-------------------------------------------------------------------------------------//
  YcpArgLoad decl(__FUNCTION__);

  InstSrcManager::ISrcId & source_id( decl.arg<YT_INTEGER, InstSrcManager::ISrcId>() );
  Url &                    url      ( decl.arg<YT_STRING, Url>() );

  if ( ! decl.load( args ) ) {
    return pkgError_bad_args;
  }
  //-------------------------------------------------------------------------------------//

  if ( ! source_id )
    return pkgError( InstSrcError::E_bad_id );

  PMError err = _y2pm.instSrcManager().rewriteUrl( source_id, url );

  if ( err )
    return pkgError( err );
*/
  return YCPBoolean( true );
}

/****************************************************************************************
 * @builtin SourceInstallOrder
 *
 * @short Explicitly set an install order.
 * @param map order_map A map of 'int order : int source_id'. source_ids are expected to
 * denote known and enabled sources.
 * @return boolean
 **/
YCPValue
PkgModuleFunctions::SourceInstallOrder (const YCPMap& ord)
{
    /* TODO FIXME
  YCPList args;
  args->add (ord);

  //-------------------------------------------------------------------------------------//
  YcpArgLoad decl(__FUNCTION__);

  YCPMap & order_map( decl.arg<YT_MAP, YCPMap>() );

  if ( ! decl.load( args ) ) {
    return pkgError_bad_args;
  }
  //-------------------------------------------------------------------------------------//

  InstSrcManager::InstOrder order;
  order.reserve( order_map->size() );
  bool error = false;

  for ( YCPMapIterator it = order_map->begin(); it != order_map->end(); ++it ) {

    if ( it.value()->isInteger() ) {
      InstSrc::UniqueID uId( it.value()->asInteger()->value() );
      InstSrcManager::ISrcId source_id( _y2pm.instSrcManager().getSourceByID( uId ) );
      if ( source_id ) {
	if ( source_id->enabled() ) {
	  order.push_back( uId );  // finaly ;)

	} else {
	  y2error ("order map entry '%s:%s': source not enabled",
		    it.key()->toString().c_str(),
		    it.value()->toString().c_str() );
	  error = true;
	}
      } else {
	y2error ("order map entry '%s:%s': bad source id",
		  it.key()->toString().c_str(),
		  it.value()->toString().c_str() );
	error = true;
      }
    } else {
      y2error ("order map entry '%s:%s': integer value expected",
		it.key()->toString().c_str(),
		it.value()->toString().c_str() );
      error = true;
    }
  }
  if ( error ) {
    return pkgError( Error::E_bad_args );
  }

  // store new instorder
  _y2pm.instSrcManager().setInstOrder( order );
*/
  return YCPBoolean( true );
}

/****************************************************************************************
 * @builtin SourceCacheCopyTo
 *
 * @short Copy cache data of all installation sources to the target
 * @description
 * Copy cache data of all installation sources to the target located below 'dir'.
 * To be called at end of initial installation.
 *
 * @param string dir Root directory of target.
 * @return boolean
 **/
YCPValue
PkgModuleFunctions::SourceCacheCopyTo (const YCPString& dir)
{
    /* TODO FIXME
  YCPList args;
  args->add (dir);

  //-------------------------------------------------------------------------------------//
  YcpArgLoad decl(__FUNCTION__);

  Pathname & nroot( decl.arg<YT_STRING, Pathname>() );

  if ( ! decl.load( args ) ) {
    return pkgError_bad_args;
  }
  //-------------------------------------------------------------------------------------//

  // Install InstSrces metadata in system
  PMError err = _y2pm.instSrcManager().cacheCopyTo( nroot );
  if ( err )
    return pkgError( err );

  // Install product data of all open sources according to installation order
#warning Review product data install here and in PM.
  InstSrcManager::ISrcIdList inst_order( _y2pm.instSrcManager().instOrderSources() );

  for ( InstSrcManager::ISrcIdList::const_iterator it = inst_order.begin(); it != inst_order.end(); ++it ) {
    _y2pm.instTarget().installProduct( (*it)->descr() );
  }

  // Actually there should be no need to do this here, as Y2PM::commitPackages
  // calls Y2PM::selectionManager().installOnTarget();
  Y2PM::selectionManager().installOnTarget();
*/
  return YCPBoolean( true );
}

/****************************************************************************************
 * @builtin SourceScan
 * @short Scan a Source Media
 * @description
 * Load all InstSrces found at media_url, i.e. all sources mentioned in /media.1/products.
 * If no /media.1/products is available, InstSrc is expected to be located directly
 * below media_url (product_dir: /).
 *
 * If a product_dir is provided, only the InstSrc located at media_url/product_dir is loaded.
 *
 * In contrary to @ref SourceCreate, InstSrces are loaded into the InstSrcManager,
 * but not enabled (packages and selections are not provided to the PackageManager),
 * and the SrcIds of <b>all</b> InstSrces found are returned.
 *
 * @param string url The media to scan.
 * @optarg string product_dir Restrict scan to a certain InstSrc located in media_url/product_dir.
 *
 * @return list<integer> list of SrcIds (integer).
 **/
YCPValue
PkgModuleFunctions::SourceScan (const YCPString& media, const YCPString& pd)
{
  zypp::SourceFactory factory;
  zypp::Url url (media->value ());
  zypp::Pathname pn(pd->value ());
  
  YCPList ids;
  unsigned int id;
  
  if ( pd->value().empty() ) {
    // scan all sources
    
    zypp::SourceFactory::ProductSet products;
        
    factory.listProducts( url, products );
    
    for( zypp::SourceFactory::ProductSet::const_iterator it = products.begin();
	it != products.end() ; ++it )
    {
	try
	{
	    // create the source, use URL as the alias
	    id = zypp::SourceManager::sourceManager()->addSource(url, pn, url.asString()+pn.asString());
	    ids->add( YCPInteger(id) );
	    y2milestone("Added source %d: %s", id, (url.asString()+pn.asString()).c_str() );  
	}
	catch ( const zypp::Exception& excpt)
	{
	    y2error("SourceCreate for '%s' product '%s' has failed"
		, url.asString().c_str(), pn.asString().c_str());
#warning Report the error
	}
    }
  } else {
    y2debug("Creating source...");

    try
    {
	// create the source, use URL as the alias
	id = zypp::SourceManager::sourceManager()->addSource(url, pn, url.asString()+pn.asString());
	ids->add( YCPInteger(id) );
    }
    catch ( const zypp::Exception& excpt)
    {
	y2error("SourceCreate for '%s' product '%s' has failed"
	    , url.asString().c_str(), pn.asString().c_str());
#warning Report the error
    }
  }

  y2milestone("Found sources: %s", ids->toString().c_str() );  
  return ids;
}

/****************************************************************************************
 * @builtin SourceCreate
 *
 * @short Create a Source
 * @description
 * Load and enable all InstSrces found at media_url, i.e. all sources mentioned in /media.1/products.
 * If no /media.1/products is available, InstSrc is expected to be located directly below
 * media_url (product_dir: /).
 *
 * If a product_dir is provided, only the InstSrc located at media_url/product_dir is loaded
 * and enabled.
 *
 * @param string url The media to scan.
 * @optarg string product_dir Restrict scan to a certain InstSrc located in media_url/product_dir.
 *
 * @return integer The source_id of the first InstSrc found on the media.
 **/
YCPValue
PkgModuleFunctions::SourceCreate (const YCPString& media, const YCPString& pd)
{
    y2debug("Creating source...");

#warning Create all sources from the given media
    
    zypp::Url url(media->value());
    zypp::filesystem::Pathname pn(pd->value());

    unsigned int ret;
    
    try
    {
	// create the source, use URL as the alias
	ret = zypp::SourceManager::sourceManager()->addSource(url, pn, url.asString());
	
	zypp::Source_Ref src = zypp::SourceManager::sourceManager()->findSource(ret);

	src.enable(); 
    
	zypp_ptr->addResolvables (src.resolvables());
    }
    catch ( const zypp::Exception& excpt)
    {
	y2error("Pkg::SourceCreate has failed");
	return YCPVoid();
    }

    return YCPInteger(ret);
}

/****************************************************************************************
 * @builtin SourceSetEnabled
 *
 * @short Set the default activation state of an InsrSrc.
 * @param integer SrcId Specifies the InstSrc.
 * @param boolean enabled Default activation state of source.
 *
 * @return boolean
 **/
YCPValue
PkgModuleFunctions::SourceSetEnabled (const YCPInteger& id, const YCPBoolean& e)
{
    zypp::Source_Ref src;
    bool enabled = e->value();

    try {
	src = zypp::SourceManager::sourceManager()->findSource(id->asInteger()->value());
    } catch(...) {
	y2error ("Source ID not found: %lld", id->asInteger()->value());
	return YCPBoolean(false);
    }

    if (enabled)
    {
	src.enable();
    }
    else
    {
	src.disable();
    }

    return YCPBoolean(src.enabled() == enabled);
}

/****************************************************************************************
 * @builtin SourceSetAutorefresh
 *
 * @short Set whether this source should automaticaly refresh it's
 * meta data when it gets enabled. (default true, if not CD/DVD)
 * @param integer SrcId Specifies the InstSrc.
 * @param boolean enabled Whether autorefresh should be turned on or off.
 *
 * @return boolean
 **/
YCPValue
PkgModuleFunctions::SourceSetAutorefresh (const YCPInteger& id, const YCPBoolean& e)
{
    /* TODO FIXME
  YCPList args;
  args->add (id);
  args->add (e);

  //-------------------------------------------------------------------------------------//
  YcpArgLoad decl(__FUNCTION__);

  InstSrcManager::ISrcId & source_id( decl.arg<YT_INTEGER, InstSrcManager::ISrcId>() );
  bool &                   enabled  ( decl.arg<YT_BOOLEAN, bool>() );

  if ( ! decl.load( args ) ) {
    return pkgError_bad_args;
  }
  //-------------------------------------------------------------------------------------//

  if ( ! source_id )
    return pkgError( InstSrcError::E_bad_id );

  PMError err = _y2pm.instSrcManager().setAutorefresh( source_id, enabled );
  if ( err )
    return pkgError( err, YCPBoolean( false ) );
*/
  return YCPBoolean( true );
}

/****************************************************************************************
 * @builtin SourceFinish
 * @short Disable an Installation Source
 * @param integer SrcId Specifies the InstSrc.
 * @return boolean
 **/
YCPValue
PkgModuleFunctions::SourceFinish (const YCPInteger& id)
{
    return SourceSetEnabled(id, false);
}

/****************************************************************************************
 * @builtin SourceRefreshNow
 * @short Attempt to immediately refresh a Source
 * @description
 * The InsrSrc will be encouraged to check and refresh all metadata
 * cached on disk.
 *
 * @param integer SrcId Specifies the InstSrc.
 *
 * @return boolean
 **/
YCPValue
PkgModuleFunctions::SourceRefreshNow (const YCPInteger& id)
{
    /* TODO FIXME
  YCPList args;
  args->add (id);

  //-------------------------------------------------------------------------------------//
  YcpArgLoad decl(__FUNCTION__);

  InstSrcManager::ISrcId & source_id( decl.arg<YT_INTEGER, InstSrcManager::ISrcId>() );

  if ( ! decl.load( args ) ) {
    return pkgError_bad_args;
  }
  //-------------------------------------------------------------------------------------//

  if ( ! source_id )
    return pkgError( InstSrcError::E_bad_id );

  PMError err =_y2pm.instSrcManager().refreshSource( source_id );
  if ( err )
    return pkgError( err, YCPBoolean( false ) );
*/
  return YCPBoolean( true );
}

/****************************************************************************************
 * @builtin SourceDelete
 * @short Delete a Source
 * @description
 * Delete an InsrSrc. The InsrSrc together with all metadata cached on disk
 * is removed. The SrcId passed becomes invalid (other SrcIds stay valid).
 *
 * @param integer SrcId Specifies the InstSrc.
 *
 * @return boolean
 **/
YCPValue
PkgModuleFunctions::SourceDelete (const YCPInteger& id)
{
    try
    {
	zypp_ptr->removeResolvables(
	    zypp::SourceManager::sourceManager()->
		findSource(id->asInteger()->value()).resolvables());

	zypp::SourceManager::sourceManager()->removeSource(id->asInteger()->value());
    }
    catch (...)
    {
	y2error("Pkg::SourceDelete: Cannot remove source %lld", id->value());
	return YCPBoolean(false);
    }

    return YCPBoolean(true);
}

/****************************************************************************************
 * @builtin SourceEditGet
 *
 * @short Get state of Sources
 * @description
 * Return a list of states for all known InstSources sorted according to the
 * source priority (highest first). A source state is a map:
 * $[
 * "SrcId"	: YCPInteger,
 * "enabled"	: YCPBoolean
 * "autorefresh": YCPBoolean
 * ];
 *
 * @return list<map> list of source states (map)
 **/
YCPValue
PkgModuleFunctions::SourceEditGet ()
{
    YCPList ret;
    std::list<unsigned int> ids = zypp::SourceManager::sourceManager()->allSources();
	    
    for( std::list<unsigned int>::iterator it = ids.begin(); it != ids.end(); ++it)
    {
	zypp::Source_Ref src = zypp::SourceManager::sourceManager()->findSource(*it);
	YCPMap src_map;

	src_map->add(YCPString("SrcId"), YCPInteger(*it));
	src_map->add(YCPString("enabled"), YCPBoolean(src.enabled()));
	src_map->add(YCPString("autorefresh"), YCPBoolean(src.autorefresh()));
	
	ret->add(src_map);
    }

    return ret;
}

/****************************************************************************************
 * @builtin SourceEditSet
 *
 * @short Rearange known InstSrces rank and default state
 * @description
 * Rearange known InstSrces rank and default state according to source_states
 * (highest priority first). Known InstSrces not mentioned in source_states
 * are deleted.
 *
 * @param list source_states List of source states. Same format as returned by
 * @see SourceEditGet.
 *
 * @return boolean
 **/
YCPValue
PkgModuleFunctions::SourceEditSet (const YCPList& states)
{
    /* TODO FIXME
  YCPList args;
  args->add (states);

  //-------------------------------------------------------------------------------------//
  YcpArgLoad decl(__FUNCTION__);

  InstSrcManager::SrcStateVector & source_states( decl.arg<YT_LIST, InstSrcManager::SrcStateVector>() );

  if ( ! decl.load( args ) ) {
    return pkgError_bad_args;
  }
  //-------------------------------------------------------------------------------------//

  PMError err = _y2pm.instSrcManager().editSet( source_states );
  if ( err )
    return pkgError( err, YCPBoolean( false ) );
*/
  return YCPBoolean( true );
}

/////////////////////////////////////////////////////////////////////////////////////////
//
// DEPRECATED
//
/////////////////////////////////////////////////////////////////////////////////////////

/****************************************************************************************
 * Pkg::SourceRaisePriority (integer SrcId) -> bool
 *
 * Raise priority of source.
 *
 * @param SrcId Specifies the InstSrc.
 *
 * @return bool
 **/
YCPValue
PkgModuleFunctions::SourceRaisePriority (const YCPInteger& id)
{
    zypp::Source_Ref src;

    try {
	src = zypp::SourceManager::sourceManager()->findSource(id->asInteger()->value());
    } catch(...) {
	y2error ("Source ID not found: %lld", id->asInteger()->value());
	return YCPBoolean(false);
    }

    // raise priority by one
    src.setPriority(src.priority() + 1);

    return YCPBoolean( true );
}

/****************************************************************************************
 * Pkg::SourceLowerPriority (integer SrcId) -> void
 *
 * Lower priority of source.
 *
 * @param SrcId Specifies the InstSrc.
 *
 * @return bool
 */
YCPValue
PkgModuleFunctions::SourceLowerPriority (const YCPInteger& id)
{
    zypp::Source_Ref src;

    try {
	src = zypp::SourceManager::sourceManager()->findSource(id->asInteger()->value());
    } catch(...) {
	y2error ("Source ID not found: %lld", id->asInteger()->value());
	return YCPBoolean(false);
    }

    // lower priority by one
    src.setPriority(src.priority() - 1);

    return YCPBoolean( true );
}

/****************************************************************************************
 * Pkg::SourceSaveRanks () -> boolean
 *
 * Save ranks to disk. Return true on success, false on error.
 **/
YCPValue
PkgModuleFunctions::SourceSaveRanks ()
{
    /* TODO FIXME
  YCPList args;

  //-------------------------------------------------------------------------------------//
  YcpArgLoad decl(__FUNCTION__);
  if ( ! decl.load( args ) ) {
    return pkgError_bad_args;
  }
  //-------------------------------------------------------------------------------------//

  PMError err = _y2pm.instSrcManager().setNewRanks();
  if ( err )
    return pkgError( err, YCPBoolean( false ) );
*/
  return YCPBoolean( true );
}



