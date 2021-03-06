/*
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Credentials.h
 * Handle getting and setting a process's credentials.
 * Copyright (C) 2012 Simon Newton
 */


#ifndef INCLUDE_OLA_BASE_CREDENTIALS_H_
#define INCLUDE_OLA_BASE_CREDENTIALS_H_

#include <unistd.h>
#include <string>

namespace ola {

using std::string;

// These functions wrap their POSIX counterparts.

uid_t GetUID();
uid_t GetEUID();
bool SetUID(uid_t new_uid);

gid_t GetGID();
gid_t GetEGID();
bool SetGID(uid_t new_gid);

typedef struct {
  string pw_name;
  string pw_passwd;  // no passwd for now
  uid_t pw_uid;
  gid_t pw_gid;
  string pw_dir;
  string pw_shell;
} PasswdEntry;

// These functions are only thread safe & reentrant if the underlying OS
// supports it.
bool GetPasswdName(const string &name, PasswdEntry *passwd);
bool GetPasswdUID(uid_t uid, PasswdEntry *passwd);

typedef struct {
  string gr_name;
  string gr_passwd;  // no passwd for now
  gid_t gr_gid;
  // vector<string> gr_mem;  // no members for now
} GroupEntry;

// These functions are only thread safe & reentrant if the underlying OS
// supports it.
bool GetGroupName(const string &name, GroupEntry *passwd);
bool GetGroupGID(gid_t gid, GroupEntry *passwd);
}  // namespace ola
#endif  // INCLUDE_OLA_BASE_CREDENTIALS_H_
