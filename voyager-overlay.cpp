// Generate Voyager calculator keyboard overlays
// Copyright 2023 Eric Smith <spacewar@gmail.com>
// SPDX-License-Identifier: GPL-3.0-only

#include <format>
#include <iostream>
#include <stdexcept>
#include <vector>

#include <podofo/podofo.h>
using namespace PoDoFo;

#include <boost/program_options.hpp>
namespace po = boost::program_options;


static constexpr double mm_per_in = 25.4;
static constexpr double pt_per_in = 72.0;


static constexpr double PAGE_INSET_LEFT_IN   = 0.625;
static constexpr double PAGE_INSET_RIGHT_IN  = 0.625;
static constexpr double PAGE_INSET_TOP_IN    = 0.625;
static constexpr double PAGE_INSET_BOTTOM_IN = 1.024;


struct RegistrationGeometry
{
  double inset_left_in;
  double inset_right_in;
  double inset_top_in;
  double inset_bottom_in;

  double square_size_in;
  double line_length_in;
  double line_width_in;
};


struct OverlayGeometry
{
  double width_in;
  double height_in;
  double corner_radius_in;

  double key_col_pitch_in;
  double key_row_pitch_in;
  double key_row_1_offset_in;

  double key_width_in;
  double key_height_in;
  double key_corner_radius_in;
};


#if 0
static std::string create_registration(double page_width_in,
				       double page_height_in,
				       const RegistrationGeometry& geom)
{
  ContentStreamString s(true);

  s.set_line_width(geom.line_width_in);
  s.set_color_space("DeviceRGB", true, true);
  s.set_color(BLACK, true, true);

  // square at top left of cut area
  s.move_to({ geom.inset_left_in,                                        page_height_in - geom.inset_top_in});	// top left
  s.rect({ geom.square_size_in, geom.square_size_in });
  s.path_close_fill_stroke();
  
  // right angle at bottom left of cut area
  s.move_to({ geom.inset_left_in,                                        geom.inset_bottom_in + geom.line_length_in });
  s.line_to({ geom.inset_left_in,                                        geom.inset_bottom_in });
  s.line_to({ geom.inset_left_in + geom.line_length_in,                  geom.inset_bottom_in });
  s.path_stroke();

  // right angle at top left of cut area
  s.move_to({ page_width_in - geom.inset_right_in - geom.line_length_in, page_height_in - geom.inset_top_in });
  s.line_to({ page_width_in - geom.inset_right_in,                       page_height_in - geom.inset_top_in });
  s.line_to({ page_width_in - geom.inset_right_in,                       page_height_in - geom.inset_top_in - geom.line_length_in});
  s.path_stroke();

  return s;
}


static std::string create_overlay(const OverlayGeometry& geom)
{
  std::string s;
  double line_width_mm = 0.1;
  double line_width_in = line_width_mm / mm_per_in;

  ContentStreamString cs(true);
  cs.set_line_width(line_width_mm / mm_per_in);
  cs.set_color(BLACK, false, true);	// set stroke color

  cs.move_to({ 0.0, geom.height_in });
  cs.rounded_rect({ geom.width_in, geom.height_in}, geom.corner_radius_in);
  cs.path_close_stroke();

  for (int row = 0; row < 4; row++)
  {
    double y = geom.height_in - row * geom.key_row_pitch_in - geom.key_row_1_offset_in;
    for (int col = 0; col < 10; col++)
    {
      if ((row == 3) && (col == 5))
	continue;  // ignore bottom half of enter key
      double key_height = geom.key_height_in;
      if ((row == 2) && (col == 5))
	key_height += geom.key_row_pitch_in;	// if top half of enter key, it's a tall key
      double x = geom.width_in / 2.0 - (5 * geom.key_col_pitch_in) + (geom.key_col_pitch_in - geom.key_width_in) / 2.0 + col * geom.key_col_pitch_in;
      cs.move_to({ x, y });
      cs.rounded_rect({ geom.key_width_in, key_height }, geom.key_corner_radius_in);
      cs.path_close_stroke();

      cs.text({ x + geom.key_width_in / 2.0, y + 0.03 },
	      HorizontalAlignment::CENTER,
	      "Hello",
	      "F1",
	      6.0 / pt_per_in);
    }
  }

  return cs;
}
#endif


constexpr RegistrationGeometry cameo4_no_mat_reg_geometry =
{
  .inset_left_in   = 0.625,
  .inset_right_in  = 0.625,
  .inset_top_in    = 0.625,
  .inset_bottom_in = 1.024,

  .square_size_in  = 0.2,
  .line_length_in  = 0.5,
  .line_width_in   = 0.5 / mm_per_in
};


static constexpr float OVERLAY_MINIMUM_Y_GAP_IN = 0.1;


#if 0
static QPDFObjectHandle createPageContents(QPDF& pdf,
					   double page_width_in,
					   double page_height_in,
					   const OverlayGeometry& geom,
					   bool show_outlines,
					   bool show_reg_marks)
{
  // Create a stream that displays our image and the given text in
  // our font.
  std::string contents;

  static double available_height_in = page_height_in - PAGE_INSET_TOP_IN - PAGE_INSET_BOTTOM_IN;
  static int y_count = available_height_in / geom.height_in;
  if ((available_height_in - ((y_count * geom.height_in) / (y_count - 1))) < OVERLAY_MINIMUM_Y_GAP_IN)
    y_count--;
  static double overlay_y_gap_in = (available_height_in - (y_count * geom.height_in)) / (y_count - 1);

  contents = ("q "                                // push graphics stack
	      + std::format("{0:g} 0 0 {0:g} 0 0 cm ", pt_per_in)	// transform to inch coordinate system, origin at top left
	      );

  if (show_reg_marks)
    contents += create_registration(page_width_in, page_height_in, cameo4_no_mat_reg_geometry);

  //for (int y = 0; y < y_count; y++)
  for (int y = 1; y < 2; y++)
  {
    double left = (page_width_in - geom.width_in) / 2.0;
    double bottom = page_height_in - PAGE_INSET_TOP_IN - ((y + 1) * geom.height_in) - (y * overlay_y_gap_in);
    contents += ("q "
		 + std::format("1 0 0 1 {0:g} {1:g} cm\n", left, bottom));

    contents += create_overlay(geom);

    contents += "Q\n";
  }

  contents += "Q\n";

  return pdf.newStream(contents);
  //return QPDFObjectHandle::newStream(&pdf, contents);
}
#endif


// PoDoFo 0.9.8 PdfPainter::Rectangle() is broken, casts roudning factors to int, and if rounding factors are small, results in no rounding
static void RoundedRectangle(PdfPainter& painter,
			     double dX,			// lower left corner
			     double dY,
			     double dWidth,
			     double dHeight,
			     double dRoundX,
			     double dRoundY)
{
  constexpr double arc_magic = 0.552284749;

  painter.MoveTo       (dX + dRoundX,                      dY);
  painter.LineTo       (dX + dWidth - dRoundX,             dY);
  painter.CubicBezierTo(dX + dWidth - dRoundX * arc_magic, dY,
			dX + dWidth,                       dY + dRoundY * arc_magic,
			dX + dWidth,                       dY + dRoundY);
  painter.LineTo       (dX + dWidth,                       dY + dHeight - dRoundY); 
  painter.CubicBezierTo(dX + dWidth,                       dY + dHeight - dRoundY * arc_magic,
			dX + dWidth - dRoundX * arc_magic, dY + dHeight,
			dX + dWidth - dRoundX,             dY + dHeight);
  painter.LineTo       (dX + dRoundX,                      dY + dHeight);
  painter.CubicBezierTo(dX + dRoundX * arc_magic,          dY + dHeight,
			dX,                                dY + dHeight - dRoundY * arc_magic,
			dX,                                dY + dHeight - dRoundY);
  painter.LineTo       (dX,                                dY + dRoundY);
  painter.CubicBezierTo(dX,                                dY + dRoundY * arc_magic,
			dX + dRoundX * arc_magic,          dY,
			dX + dRoundX,                      dY);
}


static void button(PdfPainter& painter,
		   double center_x,
		   double center_y,
		   double width,
		   double height,
		   double radius)
{
  painter.Save();
  painter.SetStrokingColor(PdfColor(0.0));
  painter.SetStrokeWidth(0.01);

#if 1
  RoundedRectangle(painter,
		   center_x - width / 2.0,
		   center_y - height / 2.0,
		   width,
		   height,
		   radius,
		   radius);
#else
  painter.Rectangle(center_x - width / 2.0,
		    center_y - height / 2.0,
		    width,
		    height,
		    radius,
		    radius);
#endif
  painter.Stroke();
  painter.Restore();
}
		   
static void create_pdf(const std::string filename,
		       const OverlayGeometry& geom,
		       bool do_outlines,
		       bool do_reg_marks)
{
  PdfStreamedDocument document(filename.c_str());
  PdfPainter painter;
  PdfPage* page;
  PdfFont* font;

  page = document.CreatePage(PdfPage::CreateStandardPageSize(ePdfPageSize_Letter));
  painter.SetPage(page);

  painter.SetTransformationMatrix(pt_per_in, 0.0, 0.0, pt_per_in, 0.0, 0.0);

  font = document.CreateFont("Aileron-Regular");
  font->SetFontSize(0.25);
  painter.SetFont(font);

  painter.DrawText(1.0, 11.0 - 1.0, "Hello, world!");

  button(painter, 4.25, 5.5, 1.0, 1.0, 0.025);

  painter.FinishPage();

  document.Close();
}


void conflicting_options(const po::variables_map& vm,
			 std::initializer_list<std::string> list,
			 bool required = false)
{
  bool found = false;
  for (std::initializer_list<std::string>::iterator it1 = list.begin();
       it1 != list.end();
       ++it1)
  {
    if (vm.count(*it1) &&
	! vm[*it1].defaulted())
      found = true;
    else
      continue;
    for (std::initializer_list<std::string>::iterator it2 = it1;
	 ++it2 != list.end();
	 )
    {
      if (vm.count(*it1) &&
	  !vm[*it1].defaulted() &&
	  vm.count(*it2) &&
	  !vm[*it2].defaulted())
	throw std::logic_error("Conflicting options `" + *it1 + "' and '" + *it2 + "'.");
    }
  }
  if (required && ! found)
    throw std::logic_error("No option in group set");
}


constexpr OverlayGeometry hp_geometry =
{
  .width_in             = 4.65,
  .height_in            = 2.10,
  .corner_radius_in     = 0.025,

  .key_col_pitch_in     = 0.45,
  .key_row_pitch_in     = 0.50,
  .key_row_1_offset_in  = 0.133,

  .key_width_in         = 0.34,
  .key_height_in        = 0.32,
  .key_corner_radius_in = 0.025
};

constexpr OverlayGeometry sm_geometry =
{
  .width_in             = 4.75,
  .height_in            = 1.95,
  .corner_radius_in     = 0.025,

  .key_col_pitch_in     = 0.475,
  .key_row_pitch_in     = 0.475,
  .key_row_1_offset_in  = 0.175,

  .key_width_in         = 0.33,
  .key_height_in        = 0.30,
  .key_corner_radius_in = 0.025
};


int main(int argc, char* argv[])
{
  std::string type;
  std::string model;
  bool do_reg_marks = false;
  bool do_outlines = false; 
  bool do_legends = false;
  const OverlayGeometry* geom = nullptr;

  try
  {
    po::options_description desc("Options");
    desc.add_options()
      ("help,h",   "output help message")
      ("cut,c",    "cut marks")
      ("print,p",  "print (registration and legends)")
      ("all,a",    "all (registration, legends, and cut marks)")
      ("hp",       "HP calculator")
      ("sm",       "Swiss Micros calculator")
      ("output,o", po::value<std::string>(), "output PDF file")
      ;

    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);

    po::notify(vm);

    if (vm.count("help"))
    {
      std::cout << desc << "\n";
      return 0;
    }

    conflicting_options(vm, {"cut", "print", "all"}, true);
    conflicting_options(vm, {"hp", "sm"});

    if (vm.count("cut"))
    {
      type = "cut";
      do_outlines  = true;
    }
    if (vm.count("print"))
    {
      type = "print";
      do_reg_marks = true;
      do_legends = true;
    }
    if (vm.count("all"))
    {
      type = "all";
      do_reg_marks = true;
      do_legends = true;
      do_outlines  = false;
    }

    if (vm.count("sm"))
    {
      model = "dm1xl";
      geom = & sm_geometry;
    }
    else
    {
      model = "voyager";
      geom = & hp_geometry;
    }
  }
  catch (std::exception& e)
  {
    std::cerr << "error: " << e.what() << "\n";
    return 1;
  }

  std::string filename = model + "-overlay-" + type + ".pdf";
  create_pdf(filename,
	     *geom,
	     do_outlines,
	     do_reg_marks);

  return 0;
}
