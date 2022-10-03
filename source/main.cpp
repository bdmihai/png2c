/*_____________________________________________________________________________
 |                                                                            |
 | COPYRIGHT (C) 2022 Mihai Baneu                                             |
 |                                                                            |
 | Permission is hereby  granted,  free of charge,  to any person obtaining a |
 | copy of this software and associated documentation files (the "Software"), |
 | to deal in the Software without restriction,  including without limitation |
 | the rights to  use, copy, modify, merge, publish, distribute,  sublicense, |
 | and/or sell copies  of  the Software, and to permit  persons to  whom  the |
 | Software is furnished to do so, subject to the following conditions:       |
 |                                                                            |
 | The above  copyright notice  and this permission notice  shall be included |
 | in all copies or substantial portions of the Software.                     |
 |                                                                            |
 | THE SOFTWARE IS PROVIDED  "AS IS",  WITHOUT WARRANTY OF ANY KIND,  EXPRESS |
 | OR   IMPLIED,   INCLUDING   BUT   NOT   LIMITED   TO   THE  WARRANTIES  OF |
 | MERCHANTABILITY,  FITNESS FOR  A  PARTICULAR  PURPOSE AND NONINFRINGEMENT. |
 | IN NO  EVENT SHALL  THE AUTHORS  OR  COPYRIGHT  HOLDERS  BE LIABLE FOR ANY |
 | CLAIM, DAMAGES OR OTHER LIABILITY,  WHETHER IN AN ACTION OF CONTRACT, TORT |
 | OR OTHERWISE, ARISING FROM,  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR  |
 | THE USE OR OTHER DEALINGS IN THE SOFTWARE.                                 |
 |____________________________________________________________________________|
 |                                                                            |
 |  Author: Mihai Baneu                           Last modified: 02.Oct.2022  |
 |                                                                            |
 |___________________________________________________________________________*/

/**
 * Original code: png2c
 * Oleg Vaskevich, Northeastern University
 * 4/17/2013
 *
 * Program to convert a PNG file to an C header as an array
 * in either RGB565 or RGB5A1 format. This is useful for
 * embedding pixmaps to display with a PAL board or Arduino.
 *
 * Thanks to: http://zarb.org/~gc/html/libpng.html
 *            http://stackoverflow.com/a/2736821/832776
 */

#include "stable.h"
#include "defines.h"

#define RGB888toRGB565(r, g, b) ((r >> 3) << 11)|((g >> 2) << 5)|((b >> 3) << 0)

int main(int argc, char *argv[])
{
  QCoreApplication app(argc, argv);

  // set the application info
  app.setApplicationName(APP_NAME);
  app.setOrganizationName(APP_COMPANY);
  app.setOrganizationDomain(APP_DOMAIN);
  app.setApplicationVersion(APP_VERSION);

  // application options
  QCommandLineParser parser;
  parser.setApplicationDescription(APP_NAME " " APP_VERSION);
  parser.addHelpOption();
  parser.addVersionOption();
  parser.addOptions({
                      {{"i", "input"  }, "Input PNG file path",                 "input"          },
                      {{"o", "output" }, "Output C file path" ,                 "output"         },
                      {{"n", "name"   }, "Name of the C structure generated"  , "image", "image" },
                      {"verbose"       , "Print debug information"                               }
                    });

  parser.process(app);

  if (!parser.isSet("input") || !parser.isSet("output")) {
    parser.showHelp();
    return 1;
  }

  if (!parser.isSet("verbose")) {
    qSetMessagePattern("%{if-info}%{message}%{endif}%{if-warning}%{message}%{endif}%{if-critical}%{message}%{endif}%{if-fatal}%{message}%{endif}");
  }

  qInfo().noquote() << "opening file" << parser.value("input").toLatin1();

  FILE *input = fopen(parser.value("input").toLatin1(), "rb");
  if (!input) {
    qCritical().noquote() << "failed to open file" << parser.value("input").toLatin1();
    return 1;
  }

  // read the header
  char header[8];
  if (fread(header, 1, 8, input) <= 0) {
    qCritical().noquote() << "failed to read png header from" << parser.value("input").toLatin1();
    return 1;
  }

  // make sure it's a PNG file
  if (png_sig_cmp((png_const_bytep)header, 0, 8)) {
    qCritical().noquote() << "file is not a valid PNG file.";
    return -1;
  }

  // create the read struct
  png_structp pPng = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
  if (!pPng) {
    qCritical().noquote() << "error: couldn't read PNG read struct.";
    return -1;
  }

  // create the info struct
  png_infop pInfo = png_create_info_struct(pPng);
  if (!pInfo) {
    qCritical().noquote() << "error: couldn't read PNG info struct.";
    png_destroy_read_struct(&pPng, (png_infopp)0, (png_infopp)0);
    return -1;
  }

  // basic error handling
  if (setjmp(png_jmpbuf(pPng))) {
    qCritical().noquote() << "error reading PNG file properties.";
    return -1;
  }

  // read PNG properties
  png_init_io(pPng, input);
  png_set_sig_bytes(pPng, 8);
  png_read_info(pPng, pInfo);
  int width = png_get_image_width(pPng, pInfo);
  int height = png_get_image_height(pPng, pInfo);
  png_byte colorType = png_get_color_type(pPng, pInfo);
  png_byte bitDepth = png_get_bit_depth(pPng, pInfo);
  int numPasses = png_set_interlace_handling(pPng);
  png_read_update_info(pPng, pInfo);
  qInfo().noquote() << QString("metadata: w: %1, h: %2, color type: %3, bit depth: %4, num passes: %5").arg(width).arg(height).arg(colorType).arg(bitDepth).arg(numPasses);


  // read the file
  if (setjmp(png_jmpbuf(pPng))) {
    qCritical().noquote() << "error reading PNG file data.";
    return -1;
  }
  png_bytep *rows = (png_bytep*) malloc(sizeof(png_bytep) * height);
  for (int y = 0; y < height; y++) {
    rows[y] = (png_byte*) malloc(png_get_rowbytes(pPng,pInfo));
  }
  png_read_image(pPng, rows);
  fclose(input);

  // verify that the PNG image is RGBA
  if (png_get_color_type(pPng, pInfo) != PNG_COLOR_TYPE_RGBA) {
    qCritical().noquote() << QString("error: Input PNG file must be RGBA (%1), but is %2.").arg(PNG_COLOR_TYPE_RGBA).arg(png_get_color_type(pPng, pInfo));
    return -1;
  }

  // create the output file
  QFile output(parser.value("output"));
  if (!output.open(QFile::Text | QFile::WriteOnly | QFile::Truncate)) {
    qCritical().noquote() << "error creating output file" << parser.value("output");
    return -1;
  }
  QTextStream out(&output);

  out << "/* Autogenerated with " << APP_NAME " " APP_VERSION << " on " << QDateTime::currentDateTime().toString() << Qt::endl;
  out << "   Input file " << QFileInfo(parser.value("input")).fileName() << Qt::endl;
  out << QString("   Metadata: w: %1, h: %2, color type: %3, bit depth: %4, num passes: %5").arg(width).arg(height).arg(colorType).arg(bitDepth).arg(numPasses) << " */ " << Qt::endl << Qt::endl;


  out << "const struct {" << Qt::endl;
  out << "  unsigned int 	 width;" << Qt::endl;
  out << "  unsigned int 	 height;" << Qt::endl;
  out << "  unsigned char	 pixel_data[];" << Qt::endl;
  out << "} " << parser.value("name") << " = {" << Qt::endl;
  out << "  " << width << ", " << height << ", " << Qt::endl;

  qInfo().noquote() << "processing pixels...";
  for (int x = 0; x < width; x++) {
    out << "  ";
    for (int y = 0; y < height; y++) {
      png_byte* row = rows[y];
      png_byte* pixel = &(row[x*4]);

      unsigned short convPixel = RGB888toRGB565(pixel[0], pixel[1], pixel[2]);
      out << QString("0x%2, 0x%1, ").arg((uint8_t)(convPixel >> 0), 2, 16, QChar('0')).arg((uint8_t)(convPixel >> 8), 2, 16, QChar('0'));
    }
    out << Qt::endl;
  }

  out << "};" << Qt::endl;

  output.close();

  qInfo().noquote() << "done!";
  return 0;
}
