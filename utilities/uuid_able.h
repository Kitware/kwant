/*ckwg +5
 * Copyright 2011 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef vidtk_uuid_able_h_
#define vidtk_uuid_able_h_

#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/signals2/mutex.hpp>

#include <vcl_string.h>
#include <vbl/vbl_smart_ptr.h>
#include <vcl_atomic_count.h>
#include "object_cache.h"

namespace vidtk
{

///
/// base class for objects with UUID identifiers
/// objects should inherit from self-templated instantiations of uuid_able
/// example:
///    class track : public uuid_able<track> {...};
/// this mixin class provides two functionalities:
///  1. provides a UUID (created on construction) for the derived object,
///     accessible through get_uuid() and set_uuid() (the latter necessary
///     for object de-serialization)
///  2. provides the reference-counting semantics required by vbl_smart_pointer.
///     this replaces the need to derive from vbl_ref_count, since to keep track
///     of objects in-core the object_cache<T> must be notified when objects
///     destruct
///

//base so we can avoid templates.
class uuid_base
{
  protected:
    static uuid_t generate()
    {
      mutex_.lock();
      boost::uint64_t current = count_;
      count_++;
      mutex_.unlock();
      //UUID is generated once and we replace the lower part with a counter
      uuid_t result = global_uuid_;
      //This should give us 16^10-1 possible uuids for the global.
      result.data[15] = current & 0x0ff;
      result.data[14] = (current>>8) & 0x0ff;
      result.data[13] = (current>>16) & 0x0ff;
      result.data[12] = (current>>24) & 0x0ff;
      result.data[11] = (current>>32) & 0x0ff;
      return result;
    }
  private:
    static uuid_t global_uuid_;
    static boost::uint64_t count_;
    static boost::signals2::mutex mutex_;
};

template<typename T>
class uuid_able : public uuid_base
{

public:
  uuid_able()
  : ref_count_(0)
  {
    // Note: Currently, placing uuid_ in the initializer list causes an
    // internal compiler error in gcc on 32 bit linux.  By moving the
    // initialization into the body of the constructor, the internal compiler
    // error is avoided.
    this->uuid_ = uuid_base::generate();//boost::uuids::random_generator()();

    // note: no add to object cache here; only add if referenced
    //  by a vbl_smart_ptr (see ref() code below)
  }

  // Copying an object should not copy the ref count.
  // (replaces functionality from vbl_ref_count)
  uuid_able(uuid_able const&u) : uuid_(u.uuid_), ref_count_(0) { }

  uuid_able( uuid_t const & u ) : uuid_(u), ref_count_(0) { }

  // (replaces functionality from vbl_ref_count)
  uuid_able&
  operator=(uuid_able const& /*rhs*/)
  { /* should not copy the ref count */ return *this; }

  virtual ~uuid_able() {}

  // get the object's UUID
  uuid_t get_uuid() const
  {
    return uuid_;
  }

  // set the object's UUID
  void set_uuid(uuid_t new_uuid)
  {
    object_cache<T> *instance = object_cache<T>::get_instance();
    instance->lock();
    instance->remove(static_cast<T*>(this));
    uuid_ = new_uuid;
    instance->add(static_cast<T*>(this));
    instance->unlock();
  }

  // regenerates the uuid.
  void regenerate_uuid()
  {
    this->uuid_ = uuid_base::generate();//boost::uuids::random_generator()();
  }

  // reference counting for use with vbl_smart_ptr
  // (replaces functionality from vbl_ref_count)
  void ref()
  {
    // add object to cache if this is the first reference to it
    if (!ref_count_)
    {
      object_cache<T> *instance = object_cache<T>::get_instance();
      instance->lock();
      instance->add(static_cast<T*>(this));
      instance->unlock();
    }
    ++ref_count_;

  }

  void unref()
  {
    /*assert(ref_count_>0);*/
    if (--ref_count_ == 0)
    {
      object_cache<T> *instance = object_cache<T>::get_instance();
      instance->lock();
      object_cache<T>::get_instance()->remove(static_cast<T*>(this));
      delete this;
      instance->unlock();
    }
  }

  static vcl_string to_string( uuid_t u )
  {
    vcl_stringstream uuid_strstrm;
    uuid_strstrm << u;
    return uuid_strstrm.str();
  }

  static void to_uuid( vcl_string s, uuid_t & u )
  {
    vcl_stringstream strm( s );
    strm >> u;
  }

  // (replaces functionality from vbl_ref_count)
  int get_references() const { return ref_count_; }
  bool is_referenced() const { return ref_count_ > 0; }

private:
  uuid_t uuid_;


  static unsigned int count_;
  vcl_atomic_count ref_count_;
};


} //end namespace vidtk
#endif // #ifndef vidtk_uuid_able_h_
