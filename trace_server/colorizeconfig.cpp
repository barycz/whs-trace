#include "colorizeconfig.h"
#include "serialize.h"
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

