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

   File:	PkgModule.h

   Author:	Klaus Kaempf <kkaempf@suse.de>
   Maintainer:  Klaus Kaempf <kkaempf@suse.de>

   Purpose:	Access to packagemanager
		Handles Pkg::function (list_of_arguments) calls
		from WFMInterpreter.
/-*/

#ifndef PkgModule_h
#define PkgModule_h

#include <PkgModuleFunctions.h>

/**
 * A simple class for package management access
 */
class PkgModule : public PkgModuleFunctions
{
  public:

    /**
     * Constructor.
     */
    PkgModule ();

    /**
     * Destructor.
     */
    virtual ~PkgModule ();

    static PkgModule* instance ();

    static void destroy();

  private:
    static PkgModule* current_pkg;

};
#endif // PkgModule_h
