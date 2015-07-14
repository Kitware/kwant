/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef vidtk_object_cache_h_
#define vidtk_object_cache_h_

#include <vcl_map.h>
#include <vbl/vbl_smart_ptr.h>

#include <boost/uuid/uuid.hpp>
#include <boost/thread/mutex.hpp>

#include <vcl_atomic_count.h>


namespace vidtk
{

template<typename T> class uuid_able;

typedef boost::uuids::uuid uuid_t;

///
/// object_cache<T> keeps track of all in-core copies of class T
/// object lifetime is controlled through smart_pointer/reference counting
/// objects in-core are looked up by UUID
///
template <typename T>
class object_cache
{
public:
  // might use boost::unordered_map (hash map) instead of vcl_map
  //  note: uuid's are *already* hashes, and this would provide O(1)
  //  instead of O(logN) access, if it matters
  typedef vcl_map<uuid_t, T*> cache_map_t;

  /// return the (only) instance of this (singleton) class
  static object_cache *get_instance()
  {
    static boost::mutex instance_lock;

    if ( ! instance_)
    {
      boost::unique_lock< boost::mutex > lock( instance_lock );
      if ( ! instance_ )
      {
        instance_ = new object_cache;
      }
    }

    return instance_;
  }

  /// Determine if object of type T with given UUID is in core
  /// returns smart pointer to object if already in-core
  /// returns NULL smart pointer if not in core
  /// database and file readers should check here before de-serializing an object
  vbl_smart_ptr<T> lookup(uuid_t const &uuid)
  {
    this->lock();

    vbl_smart_ptr<T> ptr;
    typename cache_map_t::const_iterator iter = cache_map_.find(uuid);

    if (iter != cache_map_.end())
    {
      ptr = vbl_smart_ptr<T>(iter->second);
    }
    else
    {
      ptr = static_cast<vbl_smart_ptr<T> >(NULL);
    }
    this->unlock();

    return ptr;
  }

  /// get count of objects tracked in-core (debugging)
  int get_count()
  {
    return count_;
  }

private:
  // hidden methods: class is a singleton (per templated type)
  object_cache()
    :count_(0) {};
  object_cache(object_cache const&);
  object_cache& operator=(object_cache const&);

  // pointer to the one and only one instance of this singleton class
  static object_cache *instance_;

  // mutex used to lock the object cache during updates
  boost::mutex mutex_;

  // map from UUID to memory address for in-core objects
  cache_map_t cache_map_;

  // number of objects tracked in-core (for debugging purposes)
  vcl_atomic_count count_;

  // only uuid_able<T> is able to add/remove from the cache
  friend class uuid_able<T>;

  // add a new in-core object to be tracked in the cache
  // (called when uuid_able objects construct)
  // note: locking is not done here, since the critical section in uuid_able
  //  (the only caller) must include add() and reference count manipulations, hence
  //  locking is handled in uuid_able
  void add(T* p)
  {
    uuid_t uuid = p->get_uuid();
    cache_map_[uuid] = p;
    ++count_;
  }

  // remove an object from the cache
  // (called when uuid_able objects destruct after their ref counts reach zero)
  // note: locking is not done here, since the critical section in uuid_able
  //  (the only caller) must include remove() and reference count manipulations, hence
  //  locking is handled in uuid_able
  void remove(T* p)
  {
    typename cache_map_t::iterator iter = cache_map_.find(p->get_uuid());
    if (iter != cache_map_.end())
    {
      cache_map_.erase(iter);
    }
    --count_;
  }

  void lock()
  {
    mutex_.lock();
  }

  void unlock()
  {
    mutex_.unlock();
  }
};

} // end namespace vidtk
#endif // #ifndef vidtk_object_cache_h_
