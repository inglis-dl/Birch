/*=========================================================================

  Program:  Birch (A Simple Image Viewer)
  Module:   vtkBirchQDicomTagWidget.cxx
  Language: C++

  Author: Patrick Emond <emondpd AT mcmaster DOT ca>
  Author: Dean Inglis <inglisd AT mcmaster DOT ca>

=========================================================================*/
#include <vtkBirchQDicomTagWidget.h>
#include <ui_vtkBirchQDicomTagWidget.h>

#include <gdcmReader.h>
#include <gdcmDataSet.h>
#include <gdcmSequenceOfItems.h>
#include <gdcmDict.h>
#include <gdcmDicts.h>
#include <gdcmGroupDict.h>
#include <gdcmVR.h>
#include <gdcmElement.h>
#include <gdcmGlobal.h>

#include <QTableWidgetItem>

#include <stdexcept>
#include <vector>
#include <map>
#include <sstream>

class vtkBirchQDicomTagWidgetPrivate : public Ui_vtkBirchQDicomTagWidget
{
  Q_DECLARE_PUBLIC(vtkBirchQDicomTagWidget);  

protected:
  vtkBirchQDicomTagWidget* const q_ptr;

public:
  vtkBirchQDicomTagWidgetPrivate( vtkBirchQDicomTagWidget& object );
  virtual ~vtkBirchQDicomTagWidgetPrivate(){};

  virtual void setupUi( QWidget* );
  virtual void updateUi( const std::string& fileName );

  virtual void buildDicomStrings( const std::string& fileName );
  virtual void clearDicomStrings();
  
private:
  std::vector< std::vector< std::string > > dicomStrings;

  std::string getTagString( const gdcm::Tag& t, const int& item = 0 );
  void dumpElements( const std::vector< std::pair< std::string, gdcm::DataElement > >& elemMap,
                     const gdcm::DataSet ds );
  gdcm::VR getReferenceVR( const gdcm::DataElement& de,
                           const gdcm::DictEntry& entry );
  void getElementString( std::vector< std::string >& vec, gdcm::VR& vr,
                         const gdcm::DataElement& de, 
                         const gdcm::DictEntry& entry );
  void getDataSetElements( std::vector< std::pair< std::string, gdcm::DataElement > >& elemMap,
                           const gdcm::DataSet &ds );
};

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
vtkBirchQDicomTagWidgetPrivate::vtkBirchQDicomTagWidgetPrivate(
  vtkBirchQDicomTagWidget& object ) : q_ptr(&object)
{
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void vtkBirchQDicomTagWidgetPrivate::setupUi(QWidget* widget)
{
  this->Ui_vtkBirchQDicomTagWidget::setupUi(widget);
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void vtkBirchQDicomTagWidgetPrivate::clearDicomStrings()
{
  this->dicomStrings.clear();
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void vtkBirchQDicomTagWidgetPrivate::buildDicomStrings( const std::string& fileName )
{
  gdcm::Reader reader;
  reader.SetFileName( fileName.c_str() );
  bool success = reader.Read();
  if( !success )
  {
    return;
  }
  const gdcm::File &file = reader.GetFile();
  const gdcm::DataSet &ds = file.GetDataSet();
  const gdcm::FileMetaInformation &meta = file.GetHeader();

  std::vector< std::pair< std::string, gdcm::DataElement > > elementMap1;
  std::vector< std::pair< std::string, gdcm::DataElement > > elementMap2;
  
  this->getDataSetElements( elementMap1, meta );
  this->getDataSetElements( elementMap2, ds );
  this->dumpElements( elementMap1, meta );
  this->dumpElements( elementMap2, ds );  
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void vtkBirchQDicomTagWidgetPrivate::updateUi( const std::string& fileName )
{
  this->tableWidget->clearContents();
  QStringList labels;
  labels << "Group,Element";
  labels << "VR";
  labels << "Description";
  labels << "Value";
  this->tableWidget->setHorizontalHeaderLabels(labels);
  this->tableWidget->setColumnCount( labels.size() );
  
  this->clearDicomStrings();
  if( !fileName.empty() )
  {
    this->buildDicomStrings( fileName );

    if( !this->dicomStrings.empty() )
    {
      this->tableWidget->setRowCount( this->dicomStrings.size() );
      int row = 0;
      std::vector< std::vector< std::string > >::const_iterator it;
      for( it = this->dicomStrings.begin(); it != this->dicomStrings.end(); ++it )
      {
        std::vector< std::string >::const_iterator vit;
        std::stringstream tag;        
        int i = 0;
        int col = 0;
        for( vit = (*it).begin(); vit != (*it).end(); ++vit )
        {
          if(i==0) tag << "(" << *vit << ",";
          else if(i==1) 
          {
            tag << *vit << ")";
            QTableWidgetItem* item = new QTableWidgetItem();
            item->setText( tag.str().c_str() );
            this->tableWidget->setItem( row, col++, item );   
          }
          else
          { 
            QTableWidgetItem* item = new QTableWidgetItem();
            item->setText( (*vit).c_str() );
            this->tableWidget->setItem( row, col++, item );   
          }
          i++;
        }
        row++;
      }
      this->tableWidget->resizeColumnsToContents();
    }
  }
}

// NOTE: this code is based on class gdcm/Source/MediaStorageAndFileFormat/gdcmPrinter

// given a gdcm::Tag, return either the group or the element 4 digit code as a string
//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
std::string vtkBirchQDicomTagWidgetPrivate::getTagString( const gdcm::Tag& t, const int& item )
{
  std::stringstream str;
  str.setf( std::ios::right );
  str << std::hex << std::setw( 4 ) << std::setfill( '0' ) <<
   t[ (item == 0 ? 0 : 1) ];
  return str.str();
}

// recover a vector of gdcm::DataSet elements paired to their gdcm::Tag group
//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void vtkBirchQDicomTagWidgetPrivate::getDataSetElements( 
  std::vector< std::pair< std::string, gdcm::DataElement > >& elemMap,
  const gdcm::DataSet &ds )
{
  for( gdcm::DataSet::ConstIterator it = ds.Begin();
       it != ds.End(); ++it )
  {   
    const gdcm::DataElement& de = *it;
    const gdcm::Tag& t = de.GetTag();

    std::string group = this->getTagString( t );
    elemMap.push_back( std::make_pair( group, de ) );
  }
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void vtkBirchQDicomTagWidgetPrivate::dumpElements( 
  const std::vector< std::pair< std::string, gdcm::DataElement > >& elemMap,
  const gdcm::DataSet ds )
{
  if( elemMap.empty() ) return;
  const gdcm::Global& g = gdcm::GlobalInstance;
  const gdcm::Dicts &dicts = g.GetDicts();

  std::vector< std::pair< std::string, gdcm::DataElement > >::const_iterator it;
  for( it = elemMap.begin(); it != elemMap.end(); ++it )
  {
    gdcm::DataElement de = it->second;
    gdcm::Tag t = de.GetTag();
    const char* owner = 0;
    if( t.IsPrivate() && !t.IsPrivateCreator() )
    {
      owner = ds.GetPrivateCreator( t ).c_str();
    }

    const gdcm::DictEntry& entry = dicts.GetDictEntry( t, owner );

    std::vector< std::string > vstr;

    gdcm::VR vr;
    this->getElementString( vstr, vr, de, entry );

    this->dicomStrings.push_back( vstr );

    // if we found an SQ element, recurse
    if( vr & gdcm::VR::SQ )
    {
      const gdcm::Value& value = de.GetValue();
      const gdcm::SequenceOfItems *sqi =
        dynamic_cast< const gdcm::SequenceOfItems* >( &value );

      if( sqi )
      {
        for( gdcm::SequenceOfItems::ItemVector::const_iterator sit = sqi->Items.begin();
          sit != sqi->Items.end(); ++sit )
        {
          const gdcm::Item& item = *sit;

          const gdcm::DataSet& nds = item.GetNestedDataSet();

          std::vector< std::pair< std::string, gdcm::DataElement > > elementMap;
          this->getDataSetElements( elementMap, nds );

          if( !elementMap.empty() )
          {
            this->dumpElements( elementMap, nds );
          }
        }
      }
    }
  }
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
gdcm::VR vtkBirchQDicomTagWidgetPrivate::getReferenceVR( const gdcm::DataElement& de, 
                                               const gdcm::DictEntry& entry )
{
  const gdcm::Value& value = de.GetValue();
  const gdcm::VR& vr = entry.GetVR();
  const gdcm::VR& vr_read = de.GetVR();

  gdcm::VR refvr = vr_read;

  if(  vr_read == gdcm::VR::INVALID ||
     ( vr_read == gdcm::VR::UN && vr != gdcm::VR::INVALID ) )
  {
    refvr = vr;
  }

  if( refvr.IsDual() )
  {
    refvr = gdcm::VR::UN;
  }

  if( dynamic_cast<const gdcm::SequenceOfItems*>( &value ) )
  {
    refvr = gdcm::VR::SQ;
  }

  if( refvr == gdcm::VR::INVALID )
  {
    refvr = gdcm::VR::UN;
  }

  return refvr;
}

// switch case macro used in GetElementString
#define StringFilterCase(type) \
  case gdcm::VR::type: \
  { \
    gdcm::Element< gdcm::VR::type, gdcm::VM::VM1_n > el; \
    if( !de.IsEmpty() ) \
    { \
      el.Set( de.GetValue() ); \
      if( el.GetLength() ) \
      { \
        os << el.GetValue(); \
        long l = (long) el.GetLength(); \
        for( long i = 1; i < l; ++i ) \
        { \
          os << "\\" << el.GetValue((unsigned int)i); \
        } \
      } \
      else \
      { \
        os << "(no value)"; \
      } \
    } \
  } break

// get the data element as a string: 
// tag group, tag element, VR code, dictionary name, element value
//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void vtkBirchQDicomTagWidgetPrivate::getElementString( std::vector< std::string >& vec, gdcm::VR& vr,
                       const gdcm::DataElement& de, const gdcm::DictEntry& entry )
{
  const gdcm::Value& value = de.GetValue();
  const gdcm::ByteValue *bv = de.GetByteValue();
  const gdcm::SequenceOfItems *sqi = NULL;
  const gdcm::Tag& t = de.GetTag();

  vec.push_back( this->getTagString( t ) );
  vec.push_back( this->getTagString( t, 1 ) );

  vr = this->getReferenceVR( de, entry );
  std::string vrStr = gdcm::VR::GetVRString( vr );
  vec.push_back( vrStr );

  std::string nameStr = entry.GetName();
  if( nameStr.empty() ) nameStr = "unknown";
  vec.push_back( nameStr );

  std::string valueStr = "no value";
  std::ostringstream os; 
  if( vr & gdcm::VR::VRASCII )
  {
    if( bv )
    {   
      bv->PrintASCII( os, bv->GetLength() );
    }   
  }
  else if( (vr & gdcm::VR::SQ) &&  
           ( sqi = dynamic_cast<const gdcm::SequenceOfItems*>( &value ) ) ) 
  {
    vec.push_back( valueStr );
    return;
  }
  else
  {
    switch( vr )
    {   
      StringFilterCase(AT);
      StringFilterCase(FL);
      StringFilterCase(FD);
      StringFilterCase(OF);
      StringFilterCase(SL);
      StringFilterCase(SS);
      StringFilterCase(UL);
      StringFilterCase(US);
      case gdcm::VR::INVALID:
      {   
        if( bv )
        {   
          bv->PrintASCII( os, bv->GetLength() );  
        }
      }
      default:
        os << "no value";
        break;
    }
  }

  if( !os.str().empty() ) valueStr = os.str();
  vec.push_back( valueStr );
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
vtkBirchQDicomTagWidget::vtkBirchQDicomTagWidget( QWidget* parent )
  : Superclass( parent ) , d_ptr( new vtkBirchQDicomTagWidgetPrivate( *this ) )
{
  Q_D(vtkBirchQDicomTagWidget);
  d->setupUi(this);
  d->updateUi( "" );
};

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
vtkBirchQDicomTagWidget::~vtkBirchQDicomTagWidget()
{
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void vtkBirchQDicomTagWidget::updateTableWidget( const std::string& fileName )
{
  Q_D(vtkBirchQDicomTagWidget);
  d->updateUi( fileName );
}
