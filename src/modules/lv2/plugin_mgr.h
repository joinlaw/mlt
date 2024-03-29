/*
 * JACK Rack
 *
 * Original:
 * Copyright (C) Robert Ham 2002, 2003 (node@users.sourceforge.net)
 *
 * Modification for MLT:
 * Copyright (C) 2004-2014 Meltytech, LLC
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#ifndef __JR_PLUGIN_MANAGER_H__
#define __JR_PLUGIN_MANAGER_H__

#define LADSPA_PORT_ATOM   10
#define LADSPA_IS_PORT_ATOM(x)   ((x) & LADSPA_PORT_ATOM)

#include <glib.h>

#include <lv2.h>

/* lv2 extenstions */
#include <lv2/atom/atom.h>
#include <lv2/midi/midi.h>
#include <lv2/port-groups/port-groups.h>
#include <lv2/port-props/port-props.h>
#include <lv2/presets/presets.h>
#include <lv2/resize-port/resize-port.h>
#include <lv2/ui/ui.h>
#include <lv2/worker/worker.h>

#include <lilv/lilv.h>

#include "plugin_desc.h"
#include "framework/mlt_properties.h"

typedef struct _plugin_mgr plugin_mgr_t;

struct _plugin_mgr
{
  LilvWorld *lv2_world;
  LilvPlugins *plugin_list;

  LilvNode *doap_name;
  /* LilvNode *hard_rt_capable; */

  GSList * all_plugins; 	/* this contain instances of plugin_desc_t */

  GSList * plugins;
  unsigned long plugin_count;
  mlt_properties blacklist;
};

struct _ui;

plugin_mgr_t * plugin_mgr_new ();
plugin_mgr_t * plugin_mgr2_new ();
void           plugin_mgr_destroy (plugin_mgr_t * plugin_mgr);
void           plugin_mgr2_destroy (plugin_mgr_t * plugin_mgr);

void plugin_mgr_set_plugins (plugin_mgr_t * plugin_mgr, unsigned long rack_channels);
void plugin_mgr2_set_plugins (plugin_mgr_t * plugin_mgr, unsigned long rack_channels);

plugin_desc_t * plugin_mgr_get_desc (plugin_mgr_t * plugin_mgr, unsigned long id);
plugin_desc_t * plugin_mgr2_get_desc (plugin_mgr_t * plugin_mgr, char *id);
plugin_desc_t * plugin_mgr_get_any_desc (plugin_mgr_t * plugin_mgr, unsigned long id);
plugin_desc_t * plugin_mgr2_get_any_desc (plugin_mgr_t * plugin_mgr, char *id);

#endif /* __JR_PLUGIN_MANAGER_H__ */
