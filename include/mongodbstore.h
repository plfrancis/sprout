/**
 * @file mongodbstore.h Declarations for MongDbStore class.
 *
 * Project Clearwater - IMS in the Cloud
 * Copyright (C) 2013  Metaswitch Networks Ltd
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version, along with the "Special Exception" for use of
 * the program along with SSL, set forth below. This program is distributed
 * in the hope that it will be useful, but WITHOUT ANY WARRANTY;
 * without even the implied warranty of MERCHANTABILITY or FITNESS FOR
 * A PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details. You should have received a copy of the GNU General Public
 * License along with this program.  If not, see
 * <http://www.gnu.org/licenses/>.
 *
 * The author can be reached by email at clearwater@metaswitch.com or by
 * post at Metaswitch Networks Ltd, 100 Church St, Enfield EN2 6BQ, UK
 *
 * Special Exception
 * Metaswitch Networks Ltd  grants you permission to copy, modify,
 * propagate, and distribute a work formed by combining OpenSSL with The
 * Software, or a work derivative of such a combination, even if such
 * copying, modification, propagation, or distribution would otherwise
 * violate the terms of the GPL. You must comply with the GPL in all
 * respects for all of the code used other than OpenSSL.
 * "OpenSSL" means OpenSSL toolkit software distributed by the OpenSSL
 * Project and licensed under the OpenSSL Licenses, or a work based on such
 * software and licensed under the OpenSSL Licenses.
 * "OpenSSL Licenses" means the OpenSSL License and Original SSLeay License
 * under which the OpenSSL Project distributes the OpenSSL toolkit software,
 * as those licenses appear in the file LICENSE-OPENSSL.
 */

///
/// MongoDbStore implements Store interface for storing registration data,
/// using a MongoDB cluster for storage.
///
///

#ifndef MONGODBSTORE_H__
#define MONGODBSTORE_H__

#include <sstream>

extern "C" {
#define MONGO_HAVE_STDINT
#include <bson.h>
#include <mongo.h>
}

#include "regdata.h"

namespace RegData {

/// @class RegData::MongoDbStore
///
/// A MongoDb based implementation of the Store class.
class MongoDbStore : public Store
{
public:
  MongoDbStore(const std::list<std::string>& servers);
  ~MongoDbStore();

  void flush_all();

  AoR* get_aor_data(const std::string& aor_id);
  bool set_aor_data(const std::string& aor_id, AoR* aor_data);

private:
  /// Helper: to_string method using ostringstream.
  template <class T>
    std::string to_string(T t, ///< datum to convert
                          std::ios_base & (*f)(std::ios_base&)
                          ///< modifier to apply
                         )
  {
    std::ostringstream oss;
    oss << f << t;
    return oss.str();
  }

  static std::string serialize_aor(AoR* aor_data);
  static AoR* deserialize_aor(const std::string& s);

  mongo _conn;

};

} // namespace RegData

#endif