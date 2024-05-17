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

#ifndef __LV2_PLUGIN_MANAGER_H__
#define __LV2_PLUGIN_MANAGER_H__

#define LADSPA_PORT_ATOM   10
#define LADSPA_IS_PORT_ATOM(x)   ((x) & LADSPA_PORT_ATOM)
#define LADSPA_HINT_ENUMERATION   LADSPA_HINT_DEFAULT_LOW
#define LADSPA_IS_HINT_ENUMERATION(x)     ((x) & LADSPA_HINT_ENUMERATION)

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

#include "lv2_plugin_desc.h"
#include "framework/mlt_properties.h"

typedef struct _lv2_mgr lv2_mgr_t;

struct _lv2_mgr
{
  LilvWorld *lv2_world;
  LilvPlugins *plugin_list;

  GSList * all_plugins; 	/* this contain instances of lv2_plugin_desc_t */

  GSList * plugins;
  unsigned long plugin_count;
  mlt_properties blacklist;
};

struct _ui;

lv2_mgr_t * lv2_mgr_new ();
void        lv2_mgr_destroy (lv2_mgr_t * plugin_mgr);

void lv2_mgr_set_plugins (lv2_mgr_t * plugin_mgr, unsigned long rack_channels);

lv2_plugin_desc_t * lv2_mgr_get_desc (lv2_mgr_t * plugin_mgr, char *id);
lv2_plugin_desc_t * lv2_mgr_get_any_desc (lv2_mgr_t * plugin_mgr, char *id);

#endif /* __LV2_PLUGIN_MANAGER_H__ */
