/*ckwg +5
 * Copyright 2015 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef kw_CLASS_LOADER_H_
#define kw_CLASS_LOADER_H_

#include <string>
#include <sstream>
#include <stdexcept>

#include <boost/scoped_ptr.hpp>

namespace vidtk {

// ----------------------------------------------------------------
/**
 * @brief Dynamic object loader.
 *
 * This class is a dynamic class loader that essentially converts a
 * shared library file name into a factory method.
 *
 * The factory or bootstrap function returns a new object of the
 * desired class. This bootstrap function must have a "C" interface so
 * it can be located dynamically. This function must be implemented
 * for each shared library. The default name for this function is
 * "class_bootstrap", but an alternate name can be specified in the
 * constructor. The bootstrap function is usually a factory method
 * that returns a class of the desired type.
 *
 * The function class_bootstrap_type() returns the type string of the
 * interface that is supported. This is used to check if the class
 * loader is going to create the expected object type.  In the case
 * where you are loading a class in a polymorphic structure, the type
 * returned should be that of the base interface being implemented.
 *
 * Example:
   \code
   // Define class to load logger factory.
   try
   {
      vidtk::class_loader< vidtk::logger_ns::logger_factory > logger_loader( "liblogger_gstreamer.so" );
   }
   catch (std::runtime_error &e )
   {
     std::cerr << "Error loading factory: " << e.what() << std::endl;
     return false;
   }

    vidtk::logger_ns::logger_factor fact = logger_loader.create();
    vidtk::logger_manager::instance()->set_logger_factory( fact );

   \endcode
 *
 *
 * If a different class bootstrap function name is specified, the type
 * name is returned by the function <bootstrap_func>_type().
 *
 * The function must have the following signature.
 *
  \code
  extern "C"
  {

  void* class_bootstrap()
  {
    // Return pointer to new object
    vidtk::logger_ns::logger_factory_mini_logger* ptr =  new vidtk::logger_ns::logger_factory_mini_logger;
    return ptr;
  }

  const char* class_bootstrap_type()
  {
    // Return type of interface we are implementing
    return typeid( vidtk::logger_ns::logger_factory ).name();
  }
  }
  \endcode
*/
class class_loader
{
public:
  /**
   * @brief Constructor
   *
   * The is the default constructor using the default bootstrap or
   * factory function "class_bootstrap". This function is called by
   * the create_object() method to create an object.
   *
   * @param lib_name File name of the shared object to load.
   *
   * @throws std::runtime_error
   */
  class_loader( std::string const& lib_name );

  /**
   * @brief Constructor
   *
   * This constructor uses the specified function as the bootstrap or
   * factory function for the loaded library. The bootstrap function
   * is called by the create_object() method to create an object.
   *
   * @param lib_name File name of the shared object to load.
   * @param bootstrap Name of library bootstrap function
   *
   * @throws std::runtime_error
   */
  class_loader( std::string const& lib_name, std::string const& bootstrap );

  virtual ~class_loader();

  /**
   * @brief Create new object from shared library.
   *
   * This method returns a newly created object of type T. This object
   * has been allocated from the heap.
   *
   * Use the get_object_type() method to determine the actual type of
   * objects created by the factory. Then you can decide if you have
   * the correct class loader.
   *
   * @return New object of type T
   */
  template< class T >
  T* create_object()
  {
    T* new_object = reinterpret_cast< T* >( create_object_i() );
    if ( 0 == new_object )
    {
      std::stringstream str;

      str << "class_loader:: Unable to create object";
      throw std::runtime_error( str.str() );
    }

    return new_object;
  }

  /**
   * @brief Return type name for class.
   *
   * This method returns the type name for the output of the factory
   * method. This can be used to see what type of object is being created.
   *
   * @return Type name for objects created.
   */
  std::string const& get_object_type() const;

private:

  void load( std::string const& lib_name );

  void* create_object_i();

  class impl;
  boost::scoped_ptr< impl > m_impl;

}; // end class class_loader

} // end namespace

#endif /* kw_CLASS_LOADER_H_ */
