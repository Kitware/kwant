/*ckwg +5
 * Copyright 2010,2015 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */


#ifndef _LOGGER_NS_LOGGER_FACTORY_MINI_LOGGER_H_
#define _LOGGER_NS_LOGGER_FACTORY_MINI_LOGGER_H_

#include <logger/logger_factory.h>


namespace vidtk {
namespace logger_ns {


//
// Partial types
//
class vidtk_logger_mini_logger;


// ----------------------------------------------------------------
/** Factory for underlying logger.
 *
 * This class represents the factory for the mini_logger logging service.
 *
 * An object of this type can be created early in the program
 * execution (i.e. static initializer time), which is before the
 * initialize method is called.
 */
class logger_factory_mini_logger
  : public logger_factory
{
public:
  logger_factory_mini_logger();
  virtual ~logger_factory_mini_logger();

  virtual int initialize(std::string const& config_file);

  virtual vidtk_logger_sptr get_logger( const char * const name );


}; // end class logger_factory


} // end namespace
} // end namespace

#endif /* _LOGGER_NS_LOGGER_FACTORY_MINI_LOGGER_H_ */
