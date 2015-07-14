/*ckwg +5
 * Copyright 2015 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */


#include "class_loader.h"

#include <vidtksys/DynamicLoader.hxx>
#include <boost/function.hpp>

#include <iostream>

namespace vidtk {

typedef vidtksys::DynamicLoader DL;


// ----------------------------------------------------------------
/**
 * @brief Private implementation class
 *
 */
class class_loader::impl
{
public:
  impl( std::string const& name)
    : m_bootstrap_function( name )
  {}

  ~impl()
  {}

  std::string m_bootstrap_function;

  // dynamic library handle
  vidtksys::DynamicLoader::LibraryHandle m_lib_handle;

  typedef void* (*FactoryPointer_t)();
  typedef const char* (*TypeName_t)();

  // pointer to factory function
  FactoryPointer_t m_object_factory;

  std::string m_object_type;

}; // end class class_loader_impl


// ==================================================================
class_loader::
class_loader( std::string const& lib_name )
  : m_impl( new class_loader::impl( "class_bootstrap" ) )
{
  load( lib_name );
}


class_loader::
class_loader( std::string const& lib_name, std::string const& name )
  : m_impl( new class_loader::impl( name ) )
{
  load( lib_name );
}


class_loader::
~class_loader()
{
  DL::CloseLibrary( m_impl->m_lib_handle );
}


// ------------------------------------------------------------------
void
class_loader::
load( std::string const& lib_name )
{
  m_impl->m_lib_handle = DL::OpenLibrary( lib_name.c_str() );
  if ( ! m_impl->m_lib_handle )
  {
    std::stringstream str;

    str << "class_loader::Unable to load shared library: " << DL::LastError();
    throw std::runtime_error( str.str() );
  }

  DL::SymbolPointer fp =
    DL::GetSymbolAddress( m_impl->m_lib_handle, m_impl->m_bootstrap_function.c_str() ) ;
  if ( ! fp )
  {
    std::stringstream str;

    str << "class_loader:: Unable to bind to bootstrap function( "
        << m_impl->m_bootstrap_function << "() ) : " << DL::LastError();
    throw std::runtime_error( str.str() );
  }

  m_impl->m_object_factory = reinterpret_cast< class_loader::impl::FactoryPointer_t >( fp );

  // Connect to the call which returns the class/object type
  std::string type_name = m_impl->m_bootstrap_function + "_type";
  fp = DL::GetSymbolAddress( m_impl->m_lib_handle, type_name.c_str() ) ;
  if ( ! fp )
  {
    m_impl->m_object_type = "*** unknown ***";

    std::stringstream str;

    str << "class_loader:: Unable to bind to class type function( "
        << type_name << "() ) : " << DL::LastError();
    throw std::runtime_error( str.str() );
  }
  else
  {
    class_loader::impl::TypeName_t get_type = reinterpret_cast< class_loader::impl::TypeName_t >(fp);
    m_impl->m_object_type = get_type();
  }
}


// ------------------------------------------------------------------
std::string const&
class_loader::
get_object_type() const
{
  return m_impl->m_object_type;
}


// ------------------------------------------------------------------
void*
class_loader::
create_object_i()
{
  void* new_obj = m_impl->m_object_factory();
  return new_obj;
}

} // end namespace
