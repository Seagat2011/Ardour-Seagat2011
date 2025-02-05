/*
 * Copyright (C) 2021 Paul Davis <paul@linuxaudiosystems.com>
 * Copyright (C) 2021 Ben Loftis <ben@harrisonconsoles.com>
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
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#ifndef __midi_region_trimmer_box_h__
#define __midi_region_trimmer_box_h__

#include <map>

#include <gtkmm/box.h>
#include <gtkmm/label.h>
#include <gtkmm/table.h>

#include "ardour/ardour.h"
#include "ardour/session_handle.h"

#include "gtkmm2ext/cairo_packer.h"

#include "canvas/table.h"
#include "canvas/canvas.h"
#include "canvas/rectangle.h"

#include "audio_region_trimmer_box.h"

#include "audio_clock.h"

namespace ARDOUR {
	class Session;
	class Location;
}

namespace ArdourCanvas {
	class Text;
	class Polygon;
};

class MidiTrimmerCanvas : public ArdourCanvas::Rectangle
{
  public:
	MidiTrimmerCanvas (ArdourCanvas::Item* parent);
	~MidiTrimmerCanvas ();

	void render (ArdourCanvas::Rect const & area, Cairo::RefPtr<Cairo::Context>) const;

//	void _size_allocate (ArdourCanvas::Rect const &);
	bool event_handler (GdkEvent*);
};

class MidiTrimmerBoxWidget : public ArdourCanvas::GtkCanvas
{
  public:
	MidiTrimmerBoxWidget ();
	void size_request (double& w, double& h) const;

	void on_map ();
	void on_unmap ();

  private:
	MidiTrimmerCanvas* trimmer;
};

class MidiRegionTrimmerBox : public RegionTrimmerBox
{
public:
	MidiRegionTrimmerBox ();
	~MidiRegionTrimmerBox ();

	void set_session (ARDOUR::Session*);

	void set_region (boost::shared_ptr<ARDOUR::Region>);
	void region_changed (const PBD::PropertyChange& what_changed);

private:
	Gtk::Label _header_label;
	Gtk::Table table;

	MidiTrimmerBoxWidget *trimmer_widget;

	PBD::ScopedConnection state_connection;

	boost::shared_ptr<ARDOUR::Region> _region;
};

#endif /* __midi_region_trimmer_box_h__ */
