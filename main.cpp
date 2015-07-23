
#include <QApplication>
#include <QImage>
#include <QFile>
#include <QFileInfo>
#include <QDebug>




int usage()
{
	qDebug()
		<< "Usage:\n"
		<< "  nv2rgb <file>\n";

	return 1;
}


int cannotOpenFile( const QString & filePath )
{
	qDebug()
		<< "Cannot open file: " << filePath << "\n";

	return 1;
}


int invalidSize( const int size, const int expectedSize )
{
	qDebug()
		<< "Invalid file size: " << size << ", expected size: " << expectedSize;

	return 1;
}


int invalidSize( const QString & widthString, const QString & heightString )
{
	qDebug()
		<< "Invalid size: " << widthString << "x" << heightString << "\n";

	return 1;
}


int conversionFailed()
{
	qDebug()
		<< "Convertion failed\n";

	return 1;
}


int saveFailed( const QString & filePath )
{
	qDebug()
		<< "Failed to save image into: " << filePath;

	return 1;
}


inline int real2int( const qreal x )
{
	return qBound( 0, int(x), 255 );
}


QImage convertNv12ToRgb( QIODevice * const device, const QSize & size )
{
	Q_ASSERT( device );
	Q_ASSERT( size.width() > 0 && size.height() > 0 );

	static const qreal wr = 0.299;
	static const qreal wb = 0.114;
	static const qreal wg = 1.0 - wr - wb;
	static const qreal umax = 0.436;
	static const qreal vmax = 0.615;

	const int bufferSize = size.width() * size.height() * 3 / 2;
	const QByteArray nv12 = device->read( bufferSize );
	if ( nv12.size() != bufferSize )
	{
		invalidSize( nv12.size(), bufferSize );
		return QImage();
	}

	const quint8 * const yBuffer = reinterpret_cast<const quint8 *>( nv12.constData() );
	const quint16 * const uvBuffer = reinterpret_cast<const quint16 *>( nv12.constData() + size.width()*size.height() );

	QImage image( size, QImage::Format_ARGB32 );

	for ( int y = 0; y < size.height(); ++y )
	{
		QRgb * const row = reinterpret_cast<QRgb*>( image.scanLine( y ) );
		for ( int x = 0; x < size.width(); ++x )
		{
			const int y_value = yBuffer[x + y*size.width()];
			const quint16 uv_value = uvBuffer[x/2 + y/4*size.width()];

			const int u_value = int((uv_value >> 0) & 0xff) - 128;
			const int v_value = int((uv_value >> 8) & 0xff) - 128;

			*(row + x) = qRgb(
					real2int( qreal(y_value) + v_value*(1.0 - wr)/(vmax) ),
					real2int( qreal(y_value) - u_value*(wb*(1.0 - wb)/(umax*wg)) - v_value*(wr*(1.0 - wr)/(vmax*wg)) ),
					real2int( qreal(y_value) + u_value*(1.0 - wb)/umax ) );
		}
	}

	return image;
}


int main( int argc, char ** argv )
{
	QApplication app( argc, argv );

	if ( argc <= 3 )
		return usage();

	const QString nv12FilePath = QString::fromLocal8Bit( argv[1] );
	const QString widthString = QString::fromLocal8Bit( argv[2] );
	const QString heightString = QString::fromLocal8Bit( argv[3] );

	const QSize size = QSize( widthString.toInt(), heightString.toInt() );

	if ( size.width() <= 0 || size.height() <= 0 )
		return invalidSize( widthString, heightString );

	QFile file( nv12FilePath );
	if ( !file.open( QIODevice::ReadOnly ) )
		return cannotOpenFile( nv12FilePath );

	if ( file.size() != size.width()*size.height()*3/2 )
		return invalidSize( file.size(), size.width()*size.height()*3/2 );

	const QImage image = convertNv12ToRgb( &file, size );
	if ( image.isNull() )
		return conversionFailed();

	const QFileInfo nv12FileInfo = QFileInfo( nv12FilePath );
	const QString rgbFilePath = nv12FileInfo.baseName() + ".png";
	if ( !image.save( rgbFilePath, "png" ) )
		return saveFailed( rgbFilePath );

	return 0;
}
