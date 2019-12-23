// Reads a screen image off the TFT and save to SD card.

// This sketch has been created to work with the TFT_eSPI library here:
// https://github.com/Bodmer/TFT_eSPI

// Created by: Bodmer 27/1/17
// Updated by: Bodmer 10/3/17
// Updated by: Bodmer 23/11/18 to support SDA reads and the ESP32
// Version: 0.08

// MIT licence applies, all text above must be included in derivative works

//====================================================================================
//                                  Definitions
//====================================================================================
#define BAUD_RATE 115200      // Maximum Serial Monitor rate for other messages
#define DUMP_BAUD_RATE 115200 // Rate used for screen dumps
//#define BAUD_RATE 921600      // Maximum Serial Monitor rate for other messages
//#define DUMP_BAUD_RATE 921600 // Rate used for screen dumps

#define PIXEL_TIMEOUT 100     // 100ms Time-out between pixel requests
#define START_TIMEOUT 10000   // 10s Maximum time to wait at start transfer

#define BITS_PER_PIXEL 16     // 24 for RGB colour format, 16 for 565 colour format

// File names must be alpha-numeric characters (0-9, a-z, A-Z) or "/" underscore "_"
// other ascii characters are stripped out by client, including / generates
// sub-directories
//#define DEFAULT_FILENAME "tft_screenshots/screenshot" // In case none is specified
#define DEFAULT_FILENAME "/screenshot" // In case none is specified
#define FILE_TYPE "bmp"       // jpg, bmp, png, tif are valid

// Filename extension
// '#' = add incrementing number, '@' = add timestamp, '%' add millis() timestamp,
// '*' = add nothing
// '@' and '%' will generate new unique filenames, so beware of cluttering up your
// hard drive with lots of images! The PC client sketch is set to limit the number of
// saved images to 1000 and will then prompt for a restart.
#define FILE_EXT  '@'         

// Number of pixels to send in a burst (minimum of 1), no benefit above 8
// NPIXELS values and render times:
// NPIXELS 1 = use readPixel() = >5s and 16 bit pixels only
// NPIXELS >1 using rectRead() 2 = 1.75s, 4 = 1.68s, 8 = 1.67s
#define NPIXELS 1  // Must be integer division of both TFT width and TFT height

//TFT_eSPI tft = TFT_eSPI();

//====================================================================================
//                           Screen server call with no filename
//====================================================================================
// Start a screen dump server (serial or network) - no filename specified
boolean screenServer(void)
{
  // With no filename the screenshot will be saved with a default name e.g. tft_screen_#.xxx
  // where # is a number 0-9 and xxx is a file type specified below
  return screenServer(DEFAULT_FILENAME);
}

//====================================================================================
//                           Screen server call with filename
//====================================================================================
// Start a screen dump server (serial or network) - filename specified
boolean screenServer(String filename)
{
  delay(0); // Equivalent to yield() for ESP8266;

  boolean result = bmpScreenServer(filename); // Screenshot serial port server

  delay(0); // Equivalent to yield()

  return result;
}


//====================================================================================
//                BMP file save
//====================================================================================
int sdfwrite( char* data, int len , int size , File file )
{
     for ( int i = 0; i < len; i++ )
     {
          int s = file.write( data[i] );
          if (file.getWriteError() != 0)
          {
            Serial.println("Write Error");
            Serial.println(file.getWriteError());
          }
     }
     return size;
}

boolean bmpScreenServer(String FileName)
{
 using namespace std;

// tft.setRotation(5);

 int BitDepth = 16; 
 int Width = M5.Lcd.width();
 int Height = M5.Lcd.height();

 char scfile[40];
 char filename[40];
 struct tm *timenow;

 time_t now = time(NULL);
 timenow = gmtime(&now);

 strftime(filename, sizeof(filename), "%Y%m%d%H%M%S.bmp", timenow);
 sprintf(scfile, "%s_%s", FileName.c_str(), filename);
 Serial.println(scfile);

 File file = SD.open( scfile, FILE_WRITE );
  
 // some preliminaries
 
 double dBytesPerPixel = ( (double) BitDepth ) / 8.0;
 double dBytesPerRow = dBytesPerPixel * (Width+0.0);
 dBytesPerRow = ceil(dBytesPerRow);
  
 int BytePaddingPerRow = 4 - ( (int) (dBytesPerRow) )% 4;
 if( BytePaddingPerRow == 4 )
 { BytePaddingPerRow = 0; } 
 
 double dActualBytesPerRow = dBytesPerRow + BytePaddingPerRow;
 
 double dTotalPixelBytes = Height * dActualBytesPerRow;
 
 double dPaletteSize = 0;
// if( BitDepth == 1 || BitDepth == 4 || BitDepth == 8 )
// { dPaletteSize = IntPow(2,BitDepth)*4.0; }

 // leave some room for 16-bit masks 
 if( BitDepth == 16 )
 { dPaletteSize = 3*4; }
 
 double dTotalFileSize = 14 + 40 + dPaletteSize + dTotalPixelBytes;
 
 // write the file header 

// typedef unsigned char  ebmpBYTE;
 typedef unsigned short ebmpWORD;
 typedef unsigned int  ebmpDWORD;

 ebmpWORD bfType = 19778; // BM
 ebmpDWORD bfSize = (ebmpDWORD) dTotalFileSize; 
 ebmpWORD bfReserved1 = 0; 
 ebmpWORD bfReserved2 = 0; 
 ebmpDWORD bfOffBits = (ebmpDWORD) (14+40+dPaletteSize);  
 
 sdfwrite( (char*) &(bfType) , sizeof(ebmpWORD) , 1 , file );
 sdfwrite( (char*) &(bfSize) , sizeof(ebmpDWORD) , 1 , file );
 sdfwrite( (char*) &(bfReserved1) , sizeof(ebmpWORD) , 1 , file );
 sdfwrite( (char*) &(bfReserved2) , sizeof(ebmpWORD) , 1 , file );
 sdfwrite( (char*) &(bfOffBits) , sizeof(ebmpDWORD) , 1, file );

 // write the info header 
 
 ebmpDWORD biSize = 40;
 ebmpDWORD biWidth = Width;
 ebmpDWORD biHeight = Height;
 ebmpWORD biPlanes = 1;
 ebmpWORD biBitCount = BitDepth;
 ebmpDWORD biCompression = 0;
 ebmpDWORD biSizeImage = (ebmpDWORD) dTotalPixelBytes;
 ebmpDWORD biXPelsPerMeter = 0;
 ebmpDWORD biYPelsPerMeter = 0;
 ebmpDWORD biClrUsed = 0;
 ebmpDWORD biClrImportant = 0;

 // indicates that we'll be using bit fields for 16-bit files
 if( BitDepth == 16 )
 { biCompression = 3; }
 
 sdfwrite( (char*) &(biSize) , sizeof(ebmpDWORD) , 1 , file );
 sdfwrite( (char*) &(biWidth) , sizeof(ebmpDWORD) , 1 , file );
 sdfwrite( (char*) &(biHeight) , sizeof(ebmpDWORD) , 1 , file );
 sdfwrite( (char*) &(biPlanes) , sizeof(ebmpWORD) , 1 , file );
 sdfwrite( (char*) &(biBitCount) , sizeof(ebmpWORD) , 1 , file );
 sdfwrite( (char*) &(biCompression) , sizeof(ebmpDWORD) , 1 , file );
 sdfwrite( (char*) &(biSizeImage) , sizeof(ebmpDWORD) , 1 , file );
 sdfwrite( (char*) &(biXPelsPerMeter) , sizeof(ebmpDWORD) , 1 , file );
 sdfwrite( (char*) &(biYPelsPerMeter) , sizeof(ebmpDWORD) , 1 , file ); 
 sdfwrite( (char*) &(biClrUsed) , sizeof(ebmpDWORD) , 1 , file);
 sdfwrite( (char*) &(biClrImportant) , sizeof(ebmpDWORD) , 1 , file);
 
 // write the palette 
// if( BitDepth == 1 || BitDepth == 4 || BitDepth == 8 )
// {
//  int NumberOfColors = IntPow(2,BitDepth);
  
  // if there is no palette, create one 
//  if( !Colors )
//  {
//   if( !Colors )
//   { Colors = new RGBApixel [NumberOfColors]; }
//   CreateStandardColorTable(); 
//  }
   
//  int n;
//  for( n=0 ; n < NumberOfColors ; n++ )
//  { sdfwrite( (char*) &(Colors[n]) , 4 , 1 , fp ); }
// }
 
 // write the pixels 
/*
 int i,j;
 if( BitDepth != 16 )
 {  
  ebmpBYTE* Buffer;
  int BufferSize = (int) ( (Width*BitDepth)/8.0 );
  while( 8*BufferSize < Width*BitDepth )
  { BufferSize++; }
  while( BufferSize % 4 )
  { BufferSize++; }
  
  Buffer = new ebmpBYTE [BufferSize];
  for( j=0 ; j < BufferSize; j++ )
  { Buffer[j] = 0; }
    
  j=Height-1;
  
  while( j > -1 )
  {
   bool Success = false;
   if( BitDepth == 32 )
   { Success = Write32bitRow( Buffer, BufferSize, j ); }
   if( BitDepth == 24 )
   { Success = Write24bitRow( Buffer, BufferSize, j ); }
   if( BitDepth == 8  )
   { Success = Write8bitRow( Buffer, BufferSize, j ); }
   if( BitDepth == 4  )
   { Success = Write4bitRow( Buffer, BufferSize, j ); }
   if( BitDepth == 1  )
   { Success = Write1bitRow( Buffer, BufferSize, j ); }
   if( Success )
   {
    int BytesWritten = (int) sdfwrite( (char*) Buffer, 1, BufferSize, fp );
    if( BytesWritten != BufferSize )
    { Success = false; }
   }
   if( !Success )
   {
    j = -1; 
   }
   j--; 
  }

  delete [] Buffer;
 }
 */
 
 if( BitDepth == 16 )
 {
  // write the bit masks

//  ebmpWORD BlueMask = 31;    // bits 12-16
//  ebmpWORD GreenMask = 2016; // bits 6-11
//  ebmpWORD RedMask = 63488;  // bits 1-5
  ebmpWORD GreenMask = 63488;  // bits 12-16
  ebmpWORD BlueMask = 2016;    // bits 6-11
  ebmpWORD RedMask = 31;       // bits 1-5
  ebmpWORD ZeroWORD = 0;
  
  sdfwrite( (char*) &RedMask , sizeof(ebmpWORD) , 1 , file );
  sdfwrite( (char*) &ZeroWORD , sizeof(ebmpWORD) , 1 , file );

  sdfwrite( (char*) &GreenMask , sizeof(ebmpWORD) , 1 , file );
  sdfwrite( (char*) &ZeroWORD , sizeof(ebmpWORD) , 1 , file );

  sdfwrite( (char*) &BlueMask , sizeof(ebmpWORD) , 1 , file );
  sdfwrite( (char*) &ZeroWORD , sizeof(ebmpWORD) , 1 , file );

  int DataBytes = Width*2;
  int PaddingBytes = ( 4 - DataBytes % 4 ) % 4;
  
  // write the actual pixels
  
  uint8_t color[3 * NPIXELS]; // RGB and 565 format color buffer for N pixels

  // Send all the pixels on the whole screen
  for ( uint32_t y = 0; y < M5.Lcd.height(); y++)
  {
    // Increment x by NPIXELS as we send NPIXELS for every byte received
    for ( uint32_t x = 0; x < M5.Lcd.width(); x += NPIXELS)
    {
      delay(0); // Equivalent to yield() for ESP8266;

      // Fetch N 565 format pixels from x,y and put in buffer
      uint16_t c = M5.Lcd.readPixel(x, M5.Lcd.height() - y - 1);
      color[0] = c>>8;
      color[1] = c & 0xFF;  // Swap bytes
      // Send buffer to client
      sdfwrite( (char*) color , sizeof(ebmpWORD), 1, file);
    }

  }
  
 }

 file.close();

 Serial.print("written ");
 Serial.print(dTotalPixelBytes);
 Serial.println(" bytes");
 return true;
}
