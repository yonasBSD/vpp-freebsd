/*
 * Copyright (c) 2015 Cisco and/or its affiliates.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at:
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
/*
 *------------------------------------------------------------------
 * acl_test.c - test harness plugin
 *------------------------------------------------------------------
 */

#ifdef __FreeBSD__
#include <sys/endian.h>
#endif /* __FreeBSD__ */

#include <vat/vat.h>
#include <vlibapi/api.h>
#include <vlibmemory/api.h>
#include <vppinfra/error.h>
#include <vnet/ip/ip.h>
#include <arpa/inet.h>

#include <vnet/ip/ip_format_fns.h>
#include <vnet/ethernet/ethernet_format_fns.h>

#define __plugin_msg_base acl_test_main.msg_id_base
#include <vlibapi/vat_helper_macros.h>

uword unformat_sw_if_index (unformat_input_t * input, va_list * args);

/* Declare message IDs */
#include <acl/acl.api_enum.h>
#include <acl/acl.api_types.h>
#define vl_endianfun            /* define message structures */
#include <acl/acl.api.h>
#undef vl_endianfun

typedef struct {
    /* API message ID base */
    u16 msg_id_base;
    vat_main_t *vat_main;
} acl_test_main_t;

acl_test_main_t acl_test_main;

#define foreach_reply_retval_aclindex_handler  \
_(acl_add_replace_reply) \
_(macip_acl_add_reply) \
_(macip_acl_add_replace_reply)

#define _(n)                                            \
    static void vl_api_##n##_t_handler                  \
    (vl_api_##n##_t * mp)                               \
    {                                                   \
        vat_main_t * vam = acl_test_main.vat_main;   \
        i32 retval = ntohl(mp->retval);                 \
        if (vam->async_mode) {                          \
            vam->async_errors += (retval < 0);          \
        } else {                                        \
            clib_warning("ACL index: %d", ntohl(mp->acl_index)); \
            vam->retval = retval;                       \
            vam->result_ready = 1;                      \
        }                                               \
    }
foreach_reply_retval_aclindex_handler;
#undef _

/* These two ought to be in a library somewhere but they aren't */
static uword
my_unformat_mac_address (unformat_input_t * input, va_list * args)
{
  u8 *a = va_arg (*args, u8 *);
  return unformat (input, "%x:%x:%x:%x:%x:%x", &a[0], &a[1], &a[2], &a[3],
                   &a[4], &a[5]);
}

static u8 *
my_format_mac_address (u8 * s, va_list * args)
{
  u8 *a = va_arg (*args, u8 *);
  return format (s, "%02x:%02x:%02x:%02x:%02x:%02x",
                 a[0], a[1], a[2], a[3], a[4], a[5]);
}



static void vl_api_acl_plugin_get_version_reply_t_handler
    (vl_api_acl_plugin_get_version_reply_t * mp)
    {
        vat_main_t * vam = acl_test_main.vat_main;
        clib_warning("ACL plugin version: %d.%d", ntohl(mp->major), ntohl(mp->minor));
        vam->result_ready = 1;
    }

    static void
    vl_api_acl_plugin_use_hash_lookup_get_reply_t_handler (
      vl_api_acl_plugin_use_hash_lookup_get_reply_t *mp)
    {
      vat_main_t *vam = acl_test_main.vat_main;
      clib_warning ("ACL hash lookups enabled: %d", mp->enable);
      vam->result_ready = 1;
    }

static void vl_api_acl_interface_list_details_t_handler
    (vl_api_acl_interface_list_details_t * mp)
    {
        int i;
        vat_main_t * vam = acl_test_main.vat_main;
        u8 *out = 0;
        vl_api_acl_interface_list_details_t_endian(mp);
	out = format(out, "sw_if_index: %d, count: %d, n_input: %d\n", mp->sw_if_index, mp->count, mp->n_input);
        out = format(out, "   input ");
	for(i=0; i<mp->count; i++) {
          if (i == mp->n_input)
            out = format(out, "\n  output ");
	  out = format(out, "%d ", ntohl (mp->acls[i]));
	}
        out = format(out, "\n");
        clib_warning("%s", out);
        vec_free(out);
        vam->result_ready = 1;
    }

static void vl_api_macip_acl_interface_list_details_t_handler
(vl_api_macip_acl_interface_list_details_t * mp)
{
  // NOT YET IMPLEMENTED
}


static void vl_api_acl_interface_etype_whitelist_details_t_handler
    (vl_api_acl_interface_etype_whitelist_details_t * mp)
    {
        int i;
        vat_main_t * vam = acl_test_main.vat_main;
        u8 *out = 0;
        vl_api_acl_interface_etype_whitelist_details_t_endian(mp);
	out = format(out, "sw_if_index: %d, count: %d, n_input: %d\n", mp->sw_if_index, mp->count, mp->n_input);
        out = format(out, "   input ");
	for(i=0; i<mp->count; i++) {
          if (i == mp->n_input)
            out = format(out, "\n  output ");
	  out = format(out, "%04x ", ntohs(mp->whitelist[i]));
	}
        out = format(out, "\n");
        clib_warning("%s", out);
        vec_free(out);
        vam->result_ready = 1;
    }

static void vl_api_acl_plugin_get_conn_table_max_entries_reply_t_handler
    (vl_api_acl_plugin_get_conn_table_max_entries_reply_t * mp)
    {
        vat_main_t * vam = acl_test_main.vat_main;
        clib_warning("\nConn table max entries: %d",
#ifdef __linux__
                    __bswap_64(mp->conn_table_max_entries) );
#else
                    bswap64(mp->conn_table_max_entries) );
#endif /* __linux__ */
        vam->result_ready = 1;
    }

static inline u8 *
vl_api_acl_rule_t_pretty_format (u8 *out, vl_api_acl_rule_t * a)
{
  int af = a->src_prefix.address.af ? AF_INET6 : AF_INET;
  u8 src[INET6_ADDRSTRLEN];
  u8 dst[INET6_ADDRSTRLEN];
  inet_ntop(af, &a->src_prefix.address.un, (void *)src, sizeof(src));
  inet_ntop(af, &a->dst_prefix.address.un, (void *)dst, sizeof(dst));

  out = format(out, "%s action %d src %s/%d dst %s/%d proto %d sport %d-%d dport %d-%d tcpflags %d mask %d",
                     a->src_prefix.address.af ? "ipv6" : "ipv4", a->is_permit,
                     src, a->src_prefix.len,
                     dst, a->dst_prefix.len,
                     a->proto,
                     a->srcport_or_icmptype_first, a->srcport_or_icmptype_last,
	             a->dstport_or_icmpcode_first, a->dstport_or_icmpcode_last,
                     a->tcp_flags_value, a->tcp_flags_mask);
  return(out);
}



static void vl_api_acl_details_t_handler
    (vl_api_acl_details_t * mp)
    {
        int i;
        vat_main_t * vam = acl_test_main.vat_main;
        vl_api_acl_details_t_endian(mp);
        u8 *out = 0;
        out = format(0, "acl_index: %d, count: %d\n   tag {%s}\n", mp->acl_index, mp->count, mp->tag);
	for(i=0; i<mp->count; i++) {
          out = format(out, "   ");
          out = vl_api_acl_rule_t_pretty_format(out, &mp->r[i]);
          out = format(out, "%s\n", i<mp->count-1 ? "," : "");
	}
        clib_warning("%s", out);
        vec_free(out);
        vam->result_ready = 1;
    }

static inline u8 *
vl_api_macip_acl_rule_t_pretty_format (u8 *out, vl_api_macip_acl_rule_t * a)
{
  int af = a->src_prefix.address.af ? AF_INET6 : AF_INET;
  u8 src[INET6_ADDRSTRLEN];
  inet_ntop(af, &a->src_prefix.address.un, (void *)src, sizeof(src));

  out = format(out, "%s action %d ip %s/%d mac %U mask %U",
                     a->src_prefix.address.af ? "ipv6" : "ipv4", a->is_permit,
                     src, a->src_prefix.len,
                     my_format_mac_address, a->src_mac,
                     my_format_mac_address, a->src_mac_mask);
  return(out);
}


static void vl_api_macip_acl_details_t_handler
    (vl_api_macip_acl_details_t * mp)
    {
        int i;
        vat_main_t * vam = acl_test_main.vat_main;
        vl_api_macip_acl_details_t_endian(mp);
        u8 *out = format(0,"MACIP acl_index: %d, count: %d\n   tag {%s}\n", mp->acl_index, mp->count, mp->tag);
	for(i=0; i<mp->count; i++) {
          out = format(out, "   ");
          out = vl_api_macip_acl_rule_t_pretty_format(out, &mp->r[i]);
          out = format(out, "%s\n", i<mp->count-1 ? "," : "");
	}
        clib_warning("%s", out);
        vec_free(out);
        vam->result_ready = 1;
    }

static void vl_api_macip_acl_interface_get_reply_t_handler
    (vl_api_macip_acl_interface_get_reply_t * mp)
    {
        int i;
        vat_main_t * vam = acl_test_main.vat_main;
        u8 *out = format(0, "sw_if_index with MACIP ACL count: %d\n", ntohl(mp->count));
	for(i=0; i<ntohl(mp->count); i++) {
          out = format(out, "  macip_acl_interface_add_del sw_if_index %d add acl %d\n", i, ntohl(mp->acls[i]));
	}
        out = format(out, "\n");
        clib_warning("%s", out);
        vec_free(out);
        vam->result_ready = 1;
    }

static void vl_api_acl_plugin_control_ping_reply_t_handler
  (vl_api_acl_plugin_control_ping_reply_t * mp)
{
  vat_main_t *vam = &vat_main;
  i32 retval = ntohl (mp->retval);
  if (vam->async_mode)
    {
      vam->async_errors += (retval < 0);
    }
  else
    {
      vam->retval = retval;
      vam->result_ready = 1;
    }
}

static int api_acl_plugin_get_version (vat_main_t * vam)
{
    acl_test_main_t * sm = &acl_test_main;
    vl_api_acl_plugin_get_version_t * mp;
    u32 msg_size = sizeof(*mp);
    int ret;

    vam->result_ready = 0;
    mp = vl_msg_api_alloc_as_if_client(msg_size);
    clib_memset (mp, 0, msg_size);
    mp->_vl_msg_id = ntohs (VL_API_ACL_PLUGIN_GET_VERSION + sm->msg_id_base);
    mp->client_index = vam->my_client_index;

    /* send it... */
    S(mp);

    /* Wait for a reply... */
    W (ret);
    return ret;
}

static int api_macip_acl_interface_get (vat_main_t * vam)
{
    acl_test_main_t * sm = &acl_test_main;
    vl_api_acl_plugin_get_version_t * mp;
    u32 msg_size = sizeof(*mp);
    int ret;

    vam->result_ready = 0;
    mp = vl_msg_api_alloc_as_if_client(msg_size);
    clib_memset (mp, 0, msg_size);
    mp->_vl_msg_id = ntohs (VL_API_MACIP_ACL_INTERFACE_GET + sm->msg_id_base);
    mp->client_index = vam->my_client_index;

    /* send it... */
    S(mp);

    /* Wait for a reply... */
    W (ret);
    return ret;
}

#define vec_validate_acl_rules(v, idx) \
  do {                                 \
    if (vec_len(v) < idx+1) {  \
      vec_validate(v, idx); \
      v[idx].is_permit = 0x1; \
      v[idx].srcport_or_icmptype_last = 0xffff; \
      v[idx].dstport_or_icmpcode_last = 0xffff; \
    } \
  } while (0)


/* NOT YET IMPLEMENTED */
static int api_acl_plugin_control_ping (vat_main_t * vam)
{
  return 0;
}
static int api_macip_acl_interface_list_dump (vat_main_t * vam)
{
  return 0;
}

static int api_acl_add_replace (vat_main_t * vam)
{
    acl_test_main_t * sm = &acl_test_main;
    unformat_input_t * i = vam->input;
    vl_api_acl_add_replace_t * mp;
    u32 acl_index = ~0;
    u32 msg_size = sizeof (*mp); /* without the rules */

    vl_api_acl_rule_t *rules = 0;
    int rule_idx = 0;
    int n_rules = 0;
    int n_rules_override = -1;
    u32 proto = 0;
    u32 port1 = 0;
    u32 port2 = 0;
    u32 action = 0;
    u32 tcpflags, tcpmask;
    u32 src_prefix_length = 0, dst_prefix_length = 0;
    ip4_address_t src_v4address, dst_v4address;
    ip6_address_t src_v6address, dst_v6address;
    u8 *tag = 0;
    int ret;

    if (!unformat (i, "%d", &acl_index)) {
	/* Just assume -1 */
    }

    while (unformat_check_input (i) != UNFORMAT_END_OF_INPUT)
    {
        if (unformat (i, "permit+reflect"))
          {
            vec_validate_acl_rules(rules, rule_idx);
            rules[rule_idx].is_permit = 2;
          }
        else if (unformat (i, "permit"))
          {
            vec_validate_acl_rules(rules, rule_idx);
            rules[rule_idx].is_permit = 1;
          }
        else if (unformat (i, "deny"))
          {
            vec_validate_acl_rules(rules, rule_idx);
            rules[rule_idx].is_permit = 0;
          }
        else if (unformat (i, "count %d", &n_rules_override))
          {
            /* we will use this later */
          }
        else if (unformat (i, "action %d", &action))
          {
            vec_validate_acl_rules(rules, rule_idx);
            rules[rule_idx].is_permit = action;
          }
        else if (unformat (i, "src %U/%d",
         unformat_ip4_address, &src_v4address, &src_prefix_length))
          {
            vec_validate_acl_rules(rules, rule_idx);
            memcpy (rules[rule_idx].src_prefix.address.un.ip4, &src_v4address, 4);
            rules[rule_idx].src_prefix.address.af = ADDRESS_IP4;
            rules[rule_idx].src_prefix.len = src_prefix_length;
          }
        else if (unformat (i, "src %U/%d",
         unformat_ip6_address, &src_v6address, &src_prefix_length))
          {
            vec_validate_acl_rules(rules, rule_idx);
            memcpy (rules[rule_idx].src_prefix.address.un.ip6, &src_v6address, 16);
            rules[rule_idx].src_prefix.address.af = ADDRESS_IP6;
            rules[rule_idx].src_prefix.len = src_prefix_length;
          }
        else if (unformat (i, "dst %U/%d",
         unformat_ip4_address, &dst_v4address, &dst_prefix_length))
          {
            vec_validate_acl_rules(rules, rule_idx);
            memcpy (rules[rule_idx].dst_prefix.address.un.ip4, &dst_v4address, 4);
            rules[rule_idx].dst_prefix.address.af = ADDRESS_IP4;
            rules[rule_idx].dst_prefix.len = dst_prefix_length;
          }
        else if (unformat (i, "dst %U/%d",
         unformat_ip6_address, &dst_v6address, &dst_prefix_length))
          {
            vec_validate_acl_rules(rules, rule_idx);
            memcpy (rules[rule_idx].dst_prefix.address.un.ip6, &dst_v6address, 16);
            rules[rule_idx].dst_prefix.address.af = ADDRESS_IP6;
            rules[rule_idx].dst_prefix.len = dst_prefix_length;
          }
        else if (unformat (i, "sport %d-%d", &port1, &port2))
          {
            vec_validate_acl_rules(rules, rule_idx);
            rules[rule_idx].srcport_or_icmptype_first = htons(port1);
            rules[rule_idx].srcport_or_icmptype_last = htons(port2);
          }
        else if (unformat (i, "sport %d", &port1))
          {
            vec_validate_acl_rules(rules, rule_idx);
            rules[rule_idx].srcport_or_icmptype_first = htons(port1);
            rules[rule_idx].srcport_or_icmptype_last = htons(port1);
          }
        else if (unformat (i, "dport %d-%d", &port1, &port2))
          {
            vec_validate_acl_rules(rules, rule_idx);
            rules[rule_idx].dstport_or_icmpcode_first = htons(port1);
            rules[rule_idx].dstport_or_icmpcode_last = htons(port2);
          }
        else if (unformat (i, "dport %d", &port1))
          {
            vec_validate_acl_rules(rules, rule_idx);
            rules[rule_idx].dstport_or_icmpcode_first = htons(port1);
            rules[rule_idx].dstport_or_icmpcode_last = htons(port1);
          }
        else if (unformat (i, "tcpflags %d %d", &tcpflags, &tcpmask))
          {
            vec_validate_acl_rules(rules, rule_idx);
            rules[rule_idx].tcp_flags_value = tcpflags;
            rules[rule_idx].tcp_flags_mask = tcpmask;
          }
        else if (unformat (i, "tcpflags %d mask %d", &tcpflags, &tcpmask))
          {
            vec_validate_acl_rules(rules, rule_idx);
            rules[rule_idx].tcp_flags_value = tcpflags;
            rules[rule_idx].tcp_flags_mask = tcpmask;
          }
        else if (unformat (i, "proto %d", &proto))
          {
            vec_validate_acl_rules(rules, rule_idx);
            rules[rule_idx].proto = proto;
          }
        else if (unformat (i, "tag %s", &tag))
          {
          }
        else if (unformat (i, ","))
          {
            rule_idx++;
            vec_validate_acl_rules(rules, rule_idx);
          }
        else
    break;
    }

    /* Construct the API message */
    vam->result_ready = 0;

    if(rules)
      n_rules = vec_len(rules);
    else
      n_rules = 0;

    if (n_rules_override >= 0)
      n_rules = n_rules_override;

    msg_size += n_rules*sizeof(rules[0]);

    mp = vl_msg_api_alloc_as_if_client(msg_size);
    clib_memset (mp, 0, msg_size);
    mp->_vl_msg_id = ntohs (VL_API_ACL_ADD_REPLACE + sm->msg_id_base);
    mp->client_index = vam->my_client_index;
    if ((n_rules > 0) && rules)
      clib_memcpy(mp->r, rules, n_rules*sizeof (vl_api_acl_rule_t));
    if (tag)
      {
        if (vec_len(tag) >= sizeof(mp->tag))
          {
            tag[sizeof(mp->tag)-1] = 0;
	    vec_set_len (tag, sizeof (mp->tag));
	  }
	clib_memcpy (mp->tag, tag, vec_len (tag));
	vec_free (tag);
      }
    mp->acl_index = ntohl(acl_index);
    mp->count = htonl(n_rules);

    /* send it... */
    S(mp);

    /* Wait for a reply... */
    W (ret);
    return ret;
}

static int api_acl_plugin_get_conn_table_max_entries (vat_main_t * vam)
{
    acl_test_main_t * sm = &acl_test_main;
    vl_api_acl_plugin_get_conn_table_max_entries_t * mp;
    u32 msg_size = sizeof(*mp);
    int ret;

    vam->result_ready = 0;
    mp = vl_msg_api_alloc_as_if_client(msg_size);
    memset (mp, 0, msg_size);
    mp->_vl_msg_id = ntohs (VL_API_ACL_PLUGIN_GET_CONN_TABLE_MAX_ENTRIES + sm->msg_id_base);
    mp->client_index = vam->my_client_index;

    /* send it... */
    S(mp);

    /* Wait for a reply... */
    W (ret);
    return ret;
}

static int api_acl_stats_intf_counters_enable (vat_main_t * vam)
{
    acl_test_main_t * sm = &acl_test_main;
    unformat_input_t * i = vam->input;
    vl_api_acl_stats_intf_counters_enable_t * mp;
    u32 msg_size = sizeof(*mp);
    int ret;

    vam->result_ready = 0;
    mp = vl_msg_api_alloc_as_if_client(msg_size);
    memset (mp, 0, msg_size);
    mp->_vl_msg_id = ntohs (VL_API_ACL_STATS_INTF_COUNTERS_ENABLE + sm->msg_id_base);
    mp->client_index = vam->my_client_index;
    mp->enable = 1;

    while (unformat_check_input (i) != UNFORMAT_END_OF_INPUT) {
        if (unformat (i, "disable"))
            mp->enable = 0;
        else
            break;
    }

    /* send it... */
    S(mp);

    /* Wait for a reply... */
    W (ret);
    return ret;
}

static int
api_acl_plugin_use_hash_lookup_set (vat_main_t *vam)
{
  acl_test_main_t *sm = &acl_test_main;
  unformat_input_t *i = vam->input;
  vl_api_acl_plugin_use_hash_lookup_set_t *mp;
  u32 msg_size = sizeof (*mp);
  int ret;

  vam->result_ready = 0;
  mp = vl_msg_api_alloc_as_if_client (msg_size);
  memset (mp, 0, msg_size);
  mp->_vl_msg_id =
    ntohs (VL_API_ACL_PLUGIN_USE_HASH_LOOKUP_SET + sm->msg_id_base);
  mp->client_index = vam->my_client_index;
  mp->enable = 1;

  while (unformat_check_input (i) != UNFORMAT_END_OF_INPUT)
    {
      if (unformat (i, "disable"))
	mp->enable = 0;
      else if (unformat (i, "enable"))
	mp->enable = 1;
      else
	break;
    }

  /* send it... */
  S (mp);

  /* Wait for a reply... */
  W (ret);
  return ret;
}

static int
api_acl_plugin_use_hash_lookup_get (vat_main_t *vam)
{
  acl_test_main_t *sm = &acl_test_main;
  vl_api_acl_plugin_use_hash_lookup_set_t *mp;
  u32 msg_size = sizeof (*mp);
  int ret;

  vam->result_ready = 0;
  mp = vl_msg_api_alloc_as_if_client (msg_size);
  memset (mp, 0, msg_size);
  mp->_vl_msg_id =
    ntohs (VL_API_ACL_PLUGIN_USE_HASH_LOOKUP_GET + sm->msg_id_base);
  mp->client_index = vam->my_client_index;

  /* send it... */
  S (mp);

  /* Wait for a reply... */
  W (ret);
  return ret;
}

/*
 * Read the series of ACL entries from file in the following format:
 *

@0.0.0.0/1      131.179.121.0/24        0 : 65535       0 : 65535       0x00/0x00       0x0000/0x0000
@128.0.0.0/1    85.54.226.0/23  0 : 65535       0 : 65535       0x00/0x00       0x0000/0x0000
@128.0.0.0/1    85.54.48.0/23   0 : 65535       0 : 65535       0x00/0x00       0x0000/0x0000
@128.0.0.0/1    31.237.44.0/23  0 : 65535       0 : 65535       0x00/0x00       0x0000/0x0000
@0.0.0.0/1      255.84.184.0/23 0 : 65535       0 : 65535       0x00/0x00       0x0000/0x0000
@132.92.0.0/16  0.0.0.0/0       0 : 65535       0 : 65535       0x01/0xFF       0x0000/0x0000

 *
 */

static int
api_acl_add_replace_from_file (vat_main_t * vam)
{
    int ret = -1;
    unformat_input_t * input = vam->input;
    acl_test_main_t * sm = &acl_test_main;
    vl_api_acl_add_replace_t * mp;
    u32 acl_index = ~0;
    u32 msg_size = sizeof (*mp); /* without the rules */

    vl_api_acl_rule_t *rules = 0;
    int rule_idx = -1;
    int n_rules = 0;
    int is_permit = 0;
    int append_default_permit = 0;
    u32 tcpflags = 0, tcpmask = 0;
    ip4_address_t src_v4address, dst_v4address;
    int fd = -1;

    char *file_name = NULL;
    unformat_input_t file_input;

    while (unformat_check_input (input) != UNFORMAT_END_OF_INPUT)
      {
        if (unformat (input, "filename %s", &file_name))
          {
            /* we will use this later */
          }
        else if (unformat (input, "acl-index %d", &acl_index))
	  {
            /* we will try to replace an existing ACL */
	  }
        else if (unformat (input, "permit+reflect"))
	  {
	    is_permit = 2;
	  }
        else if (unformat (input, "permit"))
	  {
	    is_permit = 1;
	  }
        else if (unformat (input, "append-default-permit"))
	  {
	    append_default_permit = 1;
	  }
	else
	  break;
      }

    if (file_name == NULL)
        goto done;

    fd = open(file_name, O_RDONLY);
    if (fd < 0)
      {
        clib_warning("Could not open file '%s'", file_name);
        goto done;
      }

    /* input from file */
    input =  &file_input;
    unformat_init_clib_file(input, fd);

    unsigned sport_low, sport_high, dport_low, dport_high;
    unsigned proto, protomask;
    u32 src_prefix_length, dst_prefix_length;
    u32 unused1, unused2;

    while (unformat_check_input (input) != UNFORMAT_END_OF_INPUT)
      {
            if (!unformat(input, "@%U/%d\t%U/%d\t%d : %d\t%d : %d\t0x%x/0x%x\t0x%x/0x%x",
                                 unformat_ip4_address, &src_v4address, &src_prefix_length,
                                 unformat_ip4_address, &dst_v4address, &dst_prefix_length,
                                 &sport_low, &sport_high, &dport_low, &dport_high, &proto, &protomask, &unused1, &unused2)) {
              clib_warning("Error parsing");
              break;
            }

	    rule_idx++;
	    vec_validate_acl_rules(rules, rule_idx);

	    rules[rule_idx].is_permit = is_permit;
	    memcpy (rules[rule_idx].src_prefix.address.un.ip4, &src_v4address, 4);
            rules[rule_idx].src_prefix.address.af = ADDRESS_IP4;
	    rules[rule_idx].src_prefix.len = src_prefix_length;
	    memcpy (rules[rule_idx].dst_prefix.address.un.ip4, &dst_v4address, 4);
            rules[rule_idx].dst_prefix.address.af = ADDRESS_IP4;
	    rules[rule_idx].dst_prefix.len = dst_prefix_length;
	    rules[rule_idx].srcport_or_icmptype_first = htons(sport_low);
	    rules[rule_idx].srcport_or_icmptype_last = htons(sport_high);
	    rules[rule_idx].dstport_or_icmpcode_first = htons(dport_low);
	    rules[rule_idx].dstport_or_icmpcode_last = htons(dport_high);
	    rules[rule_idx].tcp_flags_value = tcpflags;
	    rules[rule_idx].tcp_flags_mask = tcpmask;
	    rules[rule_idx].proto = proto;

      }

    if (append_default_permit) {
	rule_idx++;
	vec_validate_acl_rules(rules, rule_idx);

	rules[rule_idx].is_permit = is_permit == 2 ? 2 : 1;

	src_v4address.data[0]=0;
	src_v4address.data[1]=0;
	src_v4address.data[2]=0;
	src_v4address.data[3]=0;
	memcpy (rules[rule_idx].src_prefix.address.un.ip4, &src_v4address, 4);
        rules[rule_idx].src_prefix.address.af = ADDRESS_IP4;
	rules[rule_idx].src_prefix.len = 0;

	dst_v4address.data[0]=0;
	dst_v4address.data[1]=0;
	dst_v4address.data[2]=0;
	dst_v4address.data[3]=0;
	memcpy (rules[rule_idx].dst_prefix.address.un.ip4, &dst_v4address, 4);
        rules[rule_idx].dst_prefix.address.af = ADDRESS_IP4;
	rules[rule_idx].dst_prefix.len = 0;

	rules[rule_idx].srcport_or_icmptype_first = htons(0);
	rules[rule_idx].srcport_or_icmptype_last = htons(65535);
	rules[rule_idx].dstport_or_icmpcode_first = htons(0);
	rules[rule_idx].dstport_or_icmpcode_last = htons(65535);
	rules[rule_idx].tcp_flags_value = 0;
	rules[rule_idx].tcp_flags_mask = 0;
	rules[rule_idx].proto = 0;
    }

    /* Construct the API message */

    vam->result_ready = 0;

    n_rules = vec_len(rules);

    msg_size += n_rules*sizeof(rules[0]);

    mp = vl_msg_api_alloc_as_if_client(msg_size);
    clib_memset (mp, 0, msg_size);
    mp->_vl_msg_id = ntohs (VL_API_ACL_ADD_REPLACE + sm->msg_id_base);
    mp->client_index = vam->my_client_index;
    if (n_rules > 0)
      clib_memcpy(mp->r, rules, n_rules*sizeof (vl_api_acl_rule_t));
    mp->acl_index = ntohl(acl_index);
    mp->count = htonl(n_rules);

    /* send it... */
    S(mp);

    /* Wait for a reply... */
    W (ret);
done:
    if (fd > 0)
      close (fd);
    vec_free(file_name);

    return ret;
}


static int api_acl_del (vat_main_t * vam)
{
    unformat_input_t * i = vam->input;
    vl_api_acl_del_t * mp;
    u32 acl_index = ~0;
    int ret;

    if (!unformat (i, "%d", &acl_index)) {
      errmsg ("missing acl index\n");
      return -99;
    }

    /* Construct the API message */
    M(ACL_DEL, mp);
    mp->acl_index = ntohl(acl_index);

    /* send it... */
    S(mp);

    /* Wait for a reply... */
    W (ret);
    return ret;
}

static int api_macip_acl_del (vat_main_t * vam)
{
    unformat_input_t * i = vam->input;
    vl_api_acl_del_t * mp;
    u32 acl_index = ~0;
    int ret;

    if (!unformat (i, "%d", &acl_index)) {
      errmsg ("missing acl index\n");
      return -99;
    }

    /* Construct the API message */
    M(MACIP_ACL_DEL, mp);
    mp->acl_index = ntohl(acl_index);

    /* send it... */
    S(mp);

    /* Wait for a reply... */
    W (ret);
    return ret;
}

static int api_acl_interface_add_del (vat_main_t * vam)
{
    unformat_input_t * i = vam->input;
    vl_api_acl_interface_add_del_t * mp;
    u32 sw_if_index = ~0;
    u32 acl_index = ~0;
    u8 is_input = 0;
    u8 is_add = 0;
    int ret;

//    acl_interface_add_del <intfc> | sw_if_index <if-idx> acl_index <acl-idx> [out] [del]

    while (unformat_check_input (i) != UNFORMAT_END_OF_INPUT)
    {
        if (unformat (i, "%d", &acl_index))
    ;
        else
    break;
    }


    /* Parse args required to build the message */
    while (unformat_check_input (i) != UNFORMAT_END_OF_INPUT) {
        if (unformat (i, "%U", unformat_sw_if_index, vam, &sw_if_index))
            ;
        else if (unformat (i, "sw_if_index %d", &sw_if_index))
            ;
        else if (unformat (i, "add"))
            is_add = 1;
        else if (unformat (i, "del"))
            is_add = 0;
        else if (unformat (i, "acl %d", &acl_index))
            ;
        else if (unformat (i, "input"))
            is_input = 1;
        else if (unformat (i, "output"))
            is_input = 0;
        else
            break;
    }

    if (sw_if_index == ~0) {
        errmsg ("missing interface name / explicit sw_if_index number \n");
        return -99;
    }

    if (acl_index == ~0) {
        errmsg ("missing ACL index\n");
        return -99;
    }



    /* Construct the API message */
    M(ACL_INTERFACE_ADD_DEL, mp);
    mp->acl_index = ntohl(acl_index);
    mp->sw_if_index = ntohl(sw_if_index);
    mp->is_add = is_add;
    mp->is_input = is_input;

    /* send it... */
    S(mp);

    /* Wait for a reply... */
    W (ret);
    return ret;
}

static int api_macip_acl_interface_add_del (vat_main_t * vam)
{
    unformat_input_t * i = vam->input;
    vl_api_macip_acl_interface_add_del_t * mp;
    u32 sw_if_index = ~0;
    u32 acl_index = ~0;
    u8 is_add = 0;
    int ret;

    /* Parse args required to build the message */
    while (unformat_check_input (i) != UNFORMAT_END_OF_INPUT) {
        if (unformat (i, "%U", unformat_sw_if_index, vam, &sw_if_index))
            ;
        else if (unformat (i, "sw_if_index %d", &sw_if_index))
            ;
        else if (unformat (i, "add"))
            is_add = 1;
        else if (unformat (i, "del"))
            is_add = 0;
        else if (unformat (i, "acl %d", &acl_index))
            ;
        else
            break;
    }

    if (sw_if_index == ~0) {
        errmsg ("missing interface name / explicit sw_if_index number \n");
        return -99;
    }

    if (acl_index == ~0) {
        errmsg ("missing ACL index\n");
        return -99;
    }



    /* Construct the API message */
    M(MACIP_ACL_INTERFACE_ADD_DEL, mp);
    mp->acl_index = ntohl(acl_index);
    mp->sw_if_index = ntohl(sw_if_index);
    mp->is_add = is_add;

    /* send it... */
    S(mp);

    /* Wait for a reply... */
    W (ret);
    return ret;
}

static int api_acl_interface_set_acl_list (vat_main_t * vam)
{
    unformat_input_t * i = vam->input;
    vl_api_acl_interface_set_acl_list_t * mp;
    u32 sw_if_index = ~0;
    u32 acl_index = ~0;
    u32 *inacls = 0;
    u32 *outacls = 0;
    u8 is_input = 0;
    int ret;

//  acl_interface_set_acl_list <intfc> | sw_if_index <if-idx> input [acl-idx list] output [acl-idx list]

    /* Parse args required to build the message */
    while (unformat_check_input (i) != UNFORMAT_END_OF_INPUT) {
        if (unformat (i, "%U", unformat_sw_if_index, vam, &sw_if_index))
            ;
        else if (unformat (i, "sw_if_index %d", &sw_if_index))
            ;
        else if (unformat (i, "%d", &acl_index))
          {
            if(is_input)
              vec_add1(inacls, htonl(acl_index));
            else
              vec_add1(outacls, htonl(acl_index));
          }
        else if (unformat (i, "acl %d", &acl_index))
            ;
        else if (unformat (i, "input"))
            is_input = 1;
        else if (unformat (i, "output"))
            is_input = 0;
        else
            break;
    }

    if (sw_if_index == ~0) {
        errmsg ("missing interface name / explicit sw_if_index number \n");
        return -99;
    }

    /* Construct the API message */
    M2(ACL_INTERFACE_SET_ACL_LIST, mp, sizeof(u32) * (vec_len(inacls) + vec_len(outacls)));
    mp->sw_if_index = ntohl(sw_if_index);
    mp->n_input = vec_len(inacls);
    mp->count = vec_len(inacls) + vec_len(outacls);
    vec_append(inacls, outacls);
    if (vec_len(inacls) > 0)
      clib_memcpy(mp->acls, inacls, vec_len(inacls)*sizeof(u32));

    /* send it... */
    S(mp);

    /* Wait for a reply... */
    W (ret);
    return ret;
}

static int api_acl_interface_set_etype_whitelist (vat_main_t * vam)
{
    unformat_input_t * i = vam->input;
    vl_api_acl_interface_set_etype_whitelist_t * mp;
    u32 sw_if_index = ~0;
    u32 ethertype = ~0;
    u16 *etypes_in = 0;
    u16 *etypes_out = 0;
    u8 is_input = 1;
    int ret;

//  acl_interface_set_etype_whitelist <intfc> | sw_if_index <if-idx> input [ethertype list] output [ethertype list]

    /* Parse args required to build the message */
    while (unformat_check_input (i) != UNFORMAT_END_OF_INPUT) {
        if (unformat (i, "%U", unformat_sw_if_index, vam, &sw_if_index))
            ;
        else if (unformat (i, "sw_if_index %d", &sw_if_index))
            ;
        else if (unformat (i, "%x", &ethertype))
          {
            ethertype = ethertype & 0xffff;
            if(is_input)
              vec_add1(etypes_in, htons(ethertype));
            else
              vec_add1(etypes_out, htons(ethertype));
          }
        else if (unformat (i, "input"))
            is_input = 1;
        else if (unformat (i, "output"))
            is_input = 0;
        else
            break;
    }

    if (sw_if_index == ~0) {
        errmsg ("missing interface name / explicit sw_if_index number \n");
        return -99;
    }

    /* Construct the API message */
    M2(ACL_INTERFACE_SET_ETYPE_WHITELIST, mp, sizeof(u32) * (vec_len(etypes_in) + vec_len(etypes_out)));
    mp->sw_if_index = ntohl(sw_if_index);
    mp->n_input = vec_len(etypes_in);
    mp->count = vec_len(etypes_in) + vec_len(etypes_out);
    vec_append(etypes_in, etypes_out);
    if (vec_len(etypes_in) > 0)
      clib_memcpy(mp->whitelist, etypes_in, vec_len(etypes_in)*sizeof(etypes_in[0]));

    /* send it... */
    S(mp);

    /* Wait for a reply... */
    W (ret);
    return ret;
}

static void
api_acl_send_control_ping(vat_main_t *vam)
{
  vl_api_acl_plugin_control_ping_t *mp_ping;

  M(ACL_PLUGIN_CONTROL_PING, mp_ping);
  S(mp_ping);
}


static int api_acl_interface_list_dump (vat_main_t * vam)
{
    unformat_input_t * i = vam->input;
    u32 sw_if_index = ~0;
    vl_api_acl_interface_list_dump_t * mp;
    int ret;

    /* Parse args required to build the message */
    while (unformat_check_input (i) != UNFORMAT_END_OF_INPUT) {
        if (unformat (i, "%U", unformat_sw_if_index, vam, &sw_if_index))
            ;
        else if (unformat (i, "sw_if_index %d", &sw_if_index))
            ;
        else
            break;
    }

    /* Construct the API message */
    M(ACL_INTERFACE_LIST_DUMP, mp);
    mp->sw_if_index = ntohl (sw_if_index);

    /* send it... */
    S(mp);

    /* Use control ping for synchronization */
    api_acl_send_control_ping(vam);

    /* Wait for a reply... */
    W (ret);
    return ret;
}

static int api_acl_dump (vat_main_t * vam)
{
    unformat_input_t * i = vam->input;
    u32 acl_index = ~0;
    vl_api_acl_dump_t * mp;
    int ret;

    /* Parse args required to build the message */
    while (unformat_check_input (i) != UNFORMAT_END_OF_INPUT) {
        if (unformat (i, "%d", &acl_index))
            ;
        else
            break;
    }

    /* Construct the API message */
    M(ACL_DUMP, mp);
    mp->acl_index = ntohl (acl_index);

    /* send it... */
    S(mp);

    /* Use control ping for synchronization */
    api_acl_send_control_ping(vam);

    /* Wait for a reply... */
    W (ret);
    return ret;
}

static int api_macip_acl_dump (vat_main_t * vam)
{
    unformat_input_t * i = vam->input;
    u32 acl_index = ~0;
    vl_api_acl_dump_t * mp;
    int ret;

    /* Parse args required to build the message */
    while (unformat_check_input (i) != UNFORMAT_END_OF_INPUT) {
        if (unformat (i, "%d", &acl_index))
            ;
        else
            break;
    }

    /* Construct the API message */
    M(MACIP_ACL_DUMP, mp);
    mp->acl_index = ntohl (acl_index);

    /* send it... */
    S(mp);

    /* Use control ping for synchronization */
    api_acl_send_control_ping(vam);

    /* Wait for a reply... */
    W (ret);
    return ret;
}

static int api_acl_interface_etype_whitelist_dump (vat_main_t * vam)
{
    unformat_input_t * i = vam->input;
    u32 sw_if_index = ~0;
    vl_api_acl_interface_etype_whitelist_dump_t * mp;
    int ret;

    /* Parse args required to build the message */
    while (unformat_check_input (i) != UNFORMAT_END_OF_INPUT) {
        if (unformat (i, "%U", unformat_sw_if_index, vam, &sw_if_index))
            ;
        else if (unformat (i, "sw_if_index %d", &sw_if_index))
            ;
        else
            break;
    }

    /* Construct the API message */
    M(ACL_INTERFACE_ETYPE_WHITELIST_DUMP, mp);
    mp->sw_if_index = ntohl (sw_if_index);

    /* send it... */
    S(mp);

    /* Use control ping for synchronization */
    api_acl_send_control_ping(vam);

    /* Wait for a reply... */
    W (ret);
    return ret;
}


#define vec_validate_macip_acl_rules(v, idx) \
  do {                                 \
    if (vec_len(v) < idx+1) {  \
      vec_validate(v, idx); \
      v[idx].is_permit = 0x1; \
    } \
  } while (0)


static int api_macip_acl_add (vat_main_t * vam)
{
    acl_test_main_t * sm = &acl_test_main;
    unformat_input_t * i = vam->input;
    vl_api_macip_acl_add_t * mp;
    u32 msg_size = sizeof (*mp); /* without the rules */

    vl_api_macip_acl_rule_t *rules = 0;
    int rule_idx = 0;
    int n_rules = 0;
    int n_rules_override = -1;
    u32 src_prefix_length = 0;
    u32 action = 0;
    ip4_address_t src_v4address;
    ip6_address_t src_v6address;
    u8 src_mac[6];
    u8 *tag = 0;
    u8 mac_mask_all_1[6] = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };
    int ret;

    while (unformat_check_input (i) != UNFORMAT_END_OF_INPUT)
    {
        if (unformat (i, "permit"))
          {
            vec_validate_macip_acl_rules(rules, rule_idx);
            rules[rule_idx].is_permit = 1;
          }
        else if (unformat (i, "deny"))
          {
            vec_validate_macip_acl_rules(rules, rule_idx);
            rules[rule_idx].is_permit = 0;
          }
        else if (unformat (i, "count %d", &n_rules_override))
          {
            /* we will use this later */
          }
        else if (unformat (i, "action %d", &action))
          {
            vec_validate_macip_acl_rules(rules, rule_idx);
            rules[rule_idx].is_permit = action;
          }
        else if (unformat (i, "ip %U/%d",
         unformat_ip4_address, &src_v4address, &src_prefix_length) ||
                 unformat (i, "ip %U",
         unformat_ip4_address, &src_v4address))
          {
            if (src_prefix_length == 0)
              src_prefix_length = 32;
            vec_validate_macip_acl_rules(rules, rule_idx);
            memcpy (rules[rule_idx].src_prefix.address.un.ip4, &src_v4address, 4);
            rules[rule_idx].src_prefix.address.af = ADDRESS_IP4;
            rules[rule_idx].src_prefix.len = src_prefix_length;
          }
        else if (unformat (i, "src"))
          {
            /* Everything in MACIP is "source" but allow this verbosity */
          }
        else if (unformat (i, "ip %U/%d",
         unformat_ip6_address, &src_v6address, &src_prefix_length) ||
                 unformat (i, "ip %U",
         unformat_ip6_address, &src_v6address))
          {
            if (src_prefix_length == 0)
              src_prefix_length = 128;
            vec_validate_macip_acl_rules(rules, rule_idx);
            memcpy (rules[rule_idx].src_prefix.address.un.ip4, &src_v6address, 4);
            rules[rule_idx].src_prefix.address.af = ADDRESS_IP6;
            rules[rule_idx].src_prefix.len = src_prefix_length;
          }
        else if (unformat (i, "mac %U",
         my_unformat_mac_address, &src_mac))
          {
            vec_validate_macip_acl_rules(rules, rule_idx);
            memcpy (rules[rule_idx].src_mac, &src_mac, 6);
            memcpy (rules[rule_idx].src_mac_mask, &mac_mask_all_1, 6);
          }
        else if (unformat (i, "mask %U",
         my_unformat_mac_address, &src_mac))
          {
            vec_validate_macip_acl_rules(rules, rule_idx);
            memcpy (rules[rule_idx].src_mac_mask, &src_mac, 6);
          }
        else if (unformat (i, "tag %s", &tag))
          {
          }
        else if (unformat (i, ","))
          {
            rule_idx++;
            vec_validate_macip_acl_rules(rules, rule_idx);
          }
        else
    break;
    }

    /* Construct the API message */
    vam->result_ready = 0;

    if(rules)
      n_rules = vec_len(rules);

    if (n_rules_override >= 0)
      n_rules = n_rules_override;

    msg_size += n_rules*sizeof(rules[0]);

    mp = vl_msg_api_alloc_as_if_client(msg_size);
    clib_memset (mp, 0, msg_size);
    mp->_vl_msg_id = ntohs (VL_API_MACIP_ACL_ADD + sm->msg_id_base);
    mp->client_index = vam->my_client_index;
    if ((n_rules > 0) && rules)
      clib_memcpy(mp->r, rules, n_rules*sizeof (mp->r[0]));
    if (tag)
      {
        if (vec_len(tag) >= sizeof(mp->tag))
          {
            tag[sizeof(mp->tag)-1] = 0;
	    vec_set_len (tag, sizeof (mp->tag));
	  }
	clib_memcpy (mp->tag, tag, vec_len (tag));
	vec_free (tag);
      }

    mp->count = htonl(n_rules);

    /* send it... */
    S(mp);

    /* Wait for a reply... */
    W (ret);
    return ret;
}

static int api_macip_acl_add_replace (vat_main_t * vam)
{
    acl_test_main_t * sm = &acl_test_main;
    unformat_input_t * i = vam->input;
    vl_api_macip_acl_add_replace_t * mp;
    u32 acl_index = ~0;
    u32 msg_size = sizeof (*mp); /* without the rules */

    vl_api_macip_acl_rule_t *rules = 0;
    int rule_idx = 0;
    int n_rules = 0;
    int n_rules_override = -1;
    u32 src_prefix_length = 0;
    u32 action = 0;
    ip4_address_t src_v4address;
    ip6_address_t src_v6address;
    u8 src_mac[6];
    u8 *tag = 0;
    u8 mac_mask_all_1[6] = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };
    int ret;

    if (!unformat (i, "%d", &acl_index)) {
        /* Just assume -1 */
    }

    while (unformat_check_input (i) != UNFORMAT_END_OF_INPUT)
    {
        if (unformat (i, "permit"))
          {
            vec_validate_macip_acl_rules(rules, rule_idx);
            rules[rule_idx].is_permit = 1;
          }
        else if (unformat (i, "deny"))
          {
            vec_validate_macip_acl_rules(rules, rule_idx);
            rules[rule_idx].is_permit = 0;
          }
        else if (unformat (i, "count %d", &n_rules_override))
          {
            /* we will use this later */
          }
        else if (unformat (i, "action %d", &action))
          {
            vec_validate_macip_acl_rules(rules, rule_idx);
            rules[rule_idx].is_permit = action;
          }
        else if (unformat (i, "ip %U/%d",
            unformat_ip4_address, &src_v4address, &src_prefix_length) ||
                   unformat (i, "ip %U",
            unformat_ip4_address, &src_v4address))
          {
              if (src_prefix_length == 0)
                src_prefix_length = 32;
              vec_validate_macip_acl_rules(rules, rule_idx);
              memcpy (rules[rule_idx].src_prefix.address.un.ip4, &src_v4address, 4);
              rules[rule_idx].src_prefix.address.af = ADDRESS_IP4;
              rules[rule_idx].src_prefix.len = src_prefix_length;
          }
        else if (unformat (i, "src"))
          {
              /* Everything in MACIP is "source" but allow this verbosity */
          }
        else if (unformat (i, "ip %U/%d",
           unformat_ip6_address, &src_v6address, &src_prefix_length) ||
                   unformat (i, "ip %U",
           unformat_ip6_address, &src_v6address))
          {
            if (src_prefix_length == 0)
             src_prefix_length = 128;
            vec_validate_macip_acl_rules(rules, rule_idx);
            memcpy (rules[rule_idx].src_prefix.address.un.ip4, &src_v6address, 4);
            rules[rule_idx].src_prefix.address.af = ADDRESS_IP6;
            rules[rule_idx].src_prefix.len = src_prefix_length;
          }
        else if (unformat (i, "mac %U",
         my_unformat_mac_address, &src_mac))
          {
            vec_validate_macip_acl_rules(rules, rule_idx);
            memcpy (rules[rule_idx].src_mac, &src_mac, 6);
            memcpy (rules[rule_idx].src_mac_mask, &mac_mask_all_1, 6);
          }
        else if (unformat (i, "mask %U",
         my_unformat_mac_address, &src_mac))
          {
            vec_validate_macip_acl_rules(rules, rule_idx);
            memcpy (rules[rule_idx].src_mac_mask, &src_mac, 6);
          }
        else if (unformat (i, "tag %s", &tag))
          {
          }
        else if (unformat (i, ","))
          {
            rule_idx++;
            vec_validate_macip_acl_rules(rules, rule_idx);
          }
        else
    break;
    }

    if (!rules)
      {
      errmsg ("rule/s required\n");
      return -99;
      }
    /* Construct the API message */
    vam->result_ready = 0;

    if(rules)
      n_rules = vec_len(rules);

    if (n_rules_override >= 0)
      n_rules = n_rules_override;

    msg_size += n_rules*sizeof(rules[0]);

    mp = vl_msg_api_alloc_as_if_client(msg_size);
    clib_memset (mp, 0, msg_size);
    mp->_vl_msg_id = ntohs (VL_API_MACIP_ACL_ADD_REPLACE + sm->msg_id_base);
    mp->client_index = vam->my_client_index;
    if ((n_rules > 0) && rules)
      clib_memcpy(mp->r, rules, n_rules*sizeof (mp->r[0]));
    if (tag)
      {
        if (vec_len(tag) >= sizeof(mp->tag))
          {
            tag[sizeof(mp->tag)-1] = 0;
	    vec_set_len (tag, sizeof (mp->tag));
	  }
	clib_memcpy (mp->tag, tag, vec_len (tag));
	vec_free (tag);
      }

    mp->acl_index = ntohl(acl_index);
    mp->count = htonl(n_rules);

    /* send it... */
    S(mp);

    /* Wait for a reply... */
    W (ret);
    return ret;
}

#define VL_API_LOCAL_SETUP_MESSAGE_ID_TABLE local_setup_message_id_table
static void local_setup_message_id_table (vat_main_t * vam)
{
  hash_set_mem (vam->function_by_name, "acl_add_replace_from_file", api_acl_add_replace_from_file);
  hash_set_mem (vam->help_by_name, "acl_add_replace_from_file",
		"filename <file> [permit] [append-default-permit]");
}

#include <acl/acl.api_test.c>
