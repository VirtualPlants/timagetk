#include <iopnm.h>

int main( int argc, char* argv[] )
{
  int b, x, y, z;
  char *buf;
  
  buf = _readPnmImage( argv[1], &x, &y, &z, &b );
  _writePnmImage( argv[2], x, y, z, b, buf );

  return( 0 );
}
