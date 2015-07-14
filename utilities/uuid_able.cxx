#include "uuid_able.h"

//static member varables
boost::uuids::uuid vidtk::uuid_base::global_uuid_ = boost::uuids::random_generator()();
boost::uint64_t vidtk::uuid_base::count_ = 0;
boost::signals2::mutex vidtk::uuid_base::mutex_;
