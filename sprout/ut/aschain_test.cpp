/**
 * @file aschain_test.cpp UT for Sprout AsChain module
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
///----------------------------------------------------------------------------

#include <string>
#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "utils.h"
#include "siptest.hpp"
#include "fakelogger.hpp"
#include "pjutils.h"
#include "stack.h"

#include "aschain.h"
#include "callservices.h"

using namespace std;
using testing::MatchesRegex;

/// Fixture for AsChainTest
class AsChainTest : public SipTest
{
public:
  FakeLogger _log;
  AsChainTable* _as_chain_table;

  static void SetUpTestCase()
  {
    SipTest::SetUpTestCase();
  }

  static void TearDownTestCase()
  {
    SipTest::TearDownTestCase();
  }

  AsChainTest() : SipTest(NULL)
  {
    _as_chain_table = new AsChainTable();
  }

  ~AsChainTest()
  {
    delete _as_chain_table; _as_chain_table = NULL;
  }
};


Ifcs simple_ifcs(int count, ...)
{
  std::string xml = R"(<?xml version="1.0" encoding="UTF-8"?><IMSSubscription><ServiceProfile><PublicIdentity><Identity>sip:5755550011@homedomain</Identity></PublicIdentity>)";



  va_list args;
  va_start(args, count);
  for (int i = 0; i < count; i++)
  {
    const char* name = va_arg(args, const char*);
    xml += R"(<InitialFilterCriteria>
                <Priority>1</Priority>
                <ApplicationServer>
                  <ServerName>)";
    xml += name;
    xml +=     R"(</ServerName>
                  <DefaultHandling>0</DefaultHandling>
                </ApplicationServer>
              </InitialFilterCriteria>)";
  }
  va_end(args);

  xml += "</ServiceProfile></IMSSubscription>";

  std::shared_ptr<rapidxml::xml_document<> > ifc_doc (new rapidxml::xml_document<>);
  ifc_doc->parse<0>(ifc_doc->allocate_string(xml.c_str()));
  return Ifcs(ifc_doc, ifc_doc->first_node("IMSSubscription")->first_node("ServiceProfile"));
}


TEST_F(AsChainTest, Basics)
{
  Ifcs ifcs1 = simple_ifcs(0);
  AsChain as_chain(_as_chain_table, SessionCase::Originating, "sip:5755550011@homedomain", true, 0, ifcs1, NULL);
  AsChainLink as_chain_link(&as_chain, 0u);

  Ifcs ifcs2 = simple_ifcs(1, "sip:pancommunicon.cw-ngv.com");
  AsChain as_chain2(_as_chain_table, SessionCase::Originating, "sip:5755550011@homedomain", true, 0, ifcs2, NULL);
  AsChainLink as_chain_link2(&as_chain2, 0u);

  Ifcs ifcs3 = simple_ifcs(2, "sip:pancommunicon.cw-ngv.com", "sip:mmtel.homedomain");
  AsChain as_chain3(_as_chain_table, SessionCase::Originating, "sip:5755550011@homedomain", true, 0, ifcs3, NULL);
  AsChainLink as_chain_link3(&as_chain3, 0u);

  EXPECT_THAT(as_chain_link.to_string(), testing::MatchesRegex("AsChain-orig\\[0x[0-9a-f]+\\]:1/0"));
  EXPECT_EQ(SessionCase::Originating, as_chain.session_case());
  EXPECT_EQ("sip:5755550011@homedomain", as_chain._served_user);

  EXPECT_TRUE(as_chain_link.complete()) << as_chain_link.to_string();
  EXPECT_FALSE(as_chain_link2.complete()) << as_chain_link2.to_string();
  EXPECT_FALSE(as_chain_link3.complete()) << as_chain_link3.to_string();

  std::string token = as_chain_link2.next_odi_token();
  AsChainLink res = _as_chain_table->lookup(token);
  EXPECT_EQ(&as_chain2, res._as_chain);
  EXPECT_EQ(1u, res._index);
  EXPECT_TRUE(res.complete());

  CallServices calls(NULL);  // Not valid, but good enough for this UT.
}

TEST_F(AsChainTest, AsInvocation)
{
  Ifcs ifcs1 = simple_ifcs(0);
  AsChain as_chain(_as_chain_table, SessionCase::Originating, "sip:5755550011@homedomain", true, 0, ifcs1, NULL);
  AsChainLink as_chain_link(&as_chain, 0u);

  Ifcs ifcs2 = simple_ifcs(1, "sip:pancommunicon.cw-ngv.com");
  AsChain as_chain2(_as_chain_table, SessionCase::Originating, "sip:5755550011@homedomain", true, 0, ifcs2, NULL);
  AsChainLink as_chain_link2(&as_chain2, 0u);

  Ifcs ifcs3 = simple_ifcs(1, "::invalid:pancommunicon.cw-ngv.com");
  AsChain as_chain3(_as_chain_table, SessionCase::Originating, "sip:5755550011@homedomain", true, 0, ifcs3, NULL);
  AsChainLink as_chain_link3(&as_chain3, 0u);

  string str("INVITE sip:5755550099@homedomain SIP/2.0\n"
             "Via: SIP/2.0/TCP 10.64.90.97:50693;rport;branch=z9hG4bKPjPtKqxhkZnvVKI2LUEWoZVFjFaqo.cOzf;alias\n"
             "Max-Forwards: 69\n"
             "From: <sip:5755550018@homedomain>;tag=13919SIPpTag0011234\n"
             "To: <sip:5755550099@homedomain>\n"
             "Contact: <sip:5755550018@10.16.62.109:58309;transport=TCP;ob>\n"
             "Call-ID: 1-13919@10.151.20.48\n"
             "CSeq: 4 INVITE\n"
             "Route: <sip:nextnode;transport=TCP;lr;orig>\n"
             "Content-Length: 0\n\n");
  pjsip_rx_data* rdata = build_rxdata(str);
  parse_rxdata(rdata);

  pjsip_tx_data* tdata = NULL;
  pj_status_t status = PJUtils::create_request_fwd(stack_data.endpt, rdata, NULL, NULL, 0, &tdata);
  ASSERT_EQ(PJ_SUCCESS, status);

  AsChainLink::Disposition disposition;
  std::string server_name;

  // Nothing to invoke. Just proceed.
  disposition = as_chain_link.on_initial_request(tdata, server_name);
  EXPECT_EQ(AsChainLink::Disposition::Complete, disposition);
  EXPECT_EQ(server_name, "");

  // Invoke external AS on originating side.
  LOG_DEBUG("ODI %s", as_chain_link2.to_string().c_str());
  disposition = as_chain_link2.on_initial_request(tdata, server_name);
  EXPECT_EQ(AsChainLink::Disposition::Skip, disposition);
  EXPECT_EQ(server_name, "sip:pancommunicon.cw-ngv.com");
}

// ++@@@ aschain.to_string
// @@@ initial request: has MMTEL, orig and term
// ++@@@ has ASs but URI is invalid.
// ++@@@ no ASs configured.- next
// ++@@@ is_mmtel
// ++@@@ get served user

