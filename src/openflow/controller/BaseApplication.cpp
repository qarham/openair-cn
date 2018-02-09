/*
 * Licensed to the OpenAirInterface (OAI) Software Alliance under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.
 * The OpenAirInterface Software Alliance licenses this file to You under
 * the Apache License, Version 2.0  (the "License"); you may not use this file
 * except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *-------------------------------------------------------------------------------
 * For more information about the OpenAirInterface (OAI) Software Alliance:
 *      contact@openairinterface.org
 */

#include <stdio.h>

#include "BaseApplication.h"
#include "service303.h"

extern "C" {
  #include "log.h"
}

using namespace fluid_msg;
namespace openflow {

void BaseApplication::event_callback(const ControllerEvent& ev,
                                     const OpenflowMessenger& messenger) {
  if (ev.get_type() == EVENT_SWITCH_UP) {
    remove_all_flows(ev.get_connection(), messenger);
    install_default_flow(ev.get_connection(), messenger);
  } else if (ev.get_type() == EVENT_ERROR) {
    handle_error_message(static_cast<const ErrorEvent&>(ev));
  }
}

void BaseApplication::install_default_flow(fluid_base::OFConnection* ofconn,
                                           const OpenflowMessenger& messenger) {
  of13::FlowMod fm = messenger.create_default_flow_mod(0, of13::OFPFC_ADD,
                                                            LOW_PRIORITY);
  // Output to next table
  of13::GoToTable inst(NEXT_TABLE);
  fm.add_instruction(inst);
  messenger.send_of_msg(fm, ofconn);
  OAILOG_DEBUG(LOG_GTPV1U, "Default table 0 flow added\n");
}

void BaseApplication::remove_all_flows(fluid_base::OFConnection* ofconn,
                                       const OpenflowMessenger& messenger) {
  of13::FlowMod fm = messenger.create_default_flow_mod(
    0,
    of13::OFPFC_DELETE,
    0);
  // match all
  fm.out_port(of13::OFPP_ANY);
  fm.out_group(of13::OFPG_ANY);
  messenger.send_of_msg(fm, ofconn);
  return;
}

void BaseApplication::handle_error_message(const ErrorEvent &ev) {
  // First 16 bits of error message are the type, second 16 bits are the code
  OAILOG_ERROR(LOG_GTPV1U,
               "Openflow error received - type: 0x%02x, code: 0x%02x\n",
               ev.get_error_type(), ev.get_error_code());
  char type_str[50];
  char code_str[50];
  sprintf(type_str, "0x%02x", ev.get_error_type());
  sprintf(code_str, "0x%02x", ev.get_error_code());
  increment_counter("openflow_error_msg", 1, 2,
                    "error_type", type_str,
                    "error_code", code_str);
}

}
