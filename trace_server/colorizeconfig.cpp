#include "colorizeconfig.h"
#include <boost/serialization/type_info_implementation.hpp>
#include <boost/archive/xml_iarchive.hpp>
#include <boost/archive/xml_oarchive.hpp>
#include <boost/serialization/utility.hpp>
#include <boost/serialization/serialization.hpp>
#include <boost/serialization/vector.hpp>
#include <serialize/ser_qt.h>
#include <serialize.h>
#include <fstream>
#include <sstream>

bool loadConfig (ColorizeConfig & config, QString const & fname)
{
	if (!::loadConfigTemplate(config, fname))
		fillDefaultConfig(config);
	return true;
}

bool saveConfig (ColorizeConfig const & config, QString const & fname)
{
	return ::saveConfigTemplate(config, fname);
}

void ColorizeConfig::clear ()
{
	fillDefaultConfig(*this);
}

void fillDefaultConfig (ColorizeConfig & config)
{
	config = ColorizeConfig();
}


