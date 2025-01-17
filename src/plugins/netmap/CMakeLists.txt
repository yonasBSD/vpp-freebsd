# SPDX-License-Identifier: Apache-2.0
# Copyright (c) 2022 Cisco and/or its affiliates.
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at:
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

if (NOT "${CMAKE_SYSTEM_NAME}" STREQUAL "FreeBSD")
  message(WARNING "Netmap is only currently support on FreeBSD  - netmap plugin disabled")
  return()
endif()

add_vpp_plugin(netmap
  SOURCES
  plugin.c
  netmap.c
  node.c
  device.c
  cli.c
  netmap_api.c

  MULTIARCH_SOURCES
  node.c
  device.c

  INSTALL_HEADERS
  netmap.h
  net_netmap.h

  API_FILES
  netmap.api
)
