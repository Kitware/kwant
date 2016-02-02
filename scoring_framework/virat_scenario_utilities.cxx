/*ckwg +5
 * Copyright 2013-2016 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "virat_scenario_utilities.h"

#include <stdexcept>

#include <track_oracle/utils/tokenizers.h>
#include <track_oracle/aries_interface/aries_interface.h>

#include <boost/lexical_cast.hpp>
#include <boost/tokenizer.hpp>

#include <tinyxml.h>

#include <vital/logger/logger.h>
static kwiver::vital::logger_handle_t main_logger( kwiver::vital::get_logger( __FILE__ ) );

using std::make_pair;
using std::map;
using std::ostringstream;
using std::pair;
using std::runtime_error;
using std::string;
using std::vector;

namespace kwiver {
namespace kwant {

namespace virat_scenario_utilities {

bool
fn_is_virat_scenario( const string& fn )
{
  vector< string > tokens = xml_tokenizer::first_n_tokens( fn, 5 );
  // an XML document is a scenario if we see the "<VIRATScenario" string.
  for (size_t i=0; i<tokens.size(); ++i)
  {
    if (tokens[i].find( "<VIRATScenario" ) != string::npos ) return true;
  }
  return false;
}

bool process( const string& fn,
              map< string, vector< size_t > >& qid2activity_map,
              timestamp_utilities::timestamp_generator_map_type& xgtf_map,
              double fps )
{
  string err_prefix( "virat_scenario_utilities::process on '" + fn + "': " );
  bool is_scenario = fn_is_virat_scenario( fn );
  if ( ! is_scenario )
  {
    LOG_ERROR( main_logger, err_prefix << "not a scenario file?" );
    return false;
  }

  TiXmlDocument doc( fn.c_str() );
  if ( ! doc.LoadFile() ) return false;

  TiXmlNode* xmlRoot = doc.RootElement();
  if ( ! xmlRoot )
  {
    LOG_ERROR( main_logger, err_prefix << "couldn't find XML root?" );
    return false;
  }

  // only interested in GroundTruth or TestCase nodes
  size_t tc_kept_count(0), tc_ignored_count(0);
  const string gtStr( "GroundTruth" );
  const string tcStr( "TestCase" );
  TiXmlNode* xmlObject = 0;
  while ( (xmlObject = xmlRoot->IterateChildren( xmlObject )) )
  {
    if (xmlObject->Value() == gtStr)
    {
      TiXmlElement* xmlE = xmlObject->ToElement();
      if ( ! xmlE )
      {
        LOG_ERROR( main_logger, err_prefix << "couldn't convert to element?" );
        return false;
      }
      const char *attr_uri = xmlE->Attribute( "uri" );
      const char *attr_start_ts = xmlE->Attribute( "start") ;
      if ( (!attr_uri) || (!attr_start_ts)) {
        LOG_ERROR( main_logger, err_prefix << "at least one of uri or start was null at " << xmlE->Row() << "");
        return false;
      }
      if ( ! timestamp_utilities::timestamp_generator_factory::from_virat_scenario( attr_uri, attr_start_ts, fps, xgtf_map ))
      {
        // assume timestamp_utilities routine logged exact error
        return false;
      }
    }
    else if (xmlObject->Value() == tcStr)
    {
      TiXmlElement* xmlE = xmlObject->ToElement();
      if ( ! xmlE )
      {
        LOG_ERROR( main_logger, "Couldn't convert xmlObject to element?" );
        return false;
      }
      const char *attr_class = xmlE->Attribute( "class" );

      // Don't count the 'CommandTestCase' stanza
      if (attr_class && (string(attr_class) == "CommandTestCase")) continue;

      bool keep_this = false;
      TiXmlHandle h( xmlObject );
      TiXmlElement* query = h.FirstChild( "VIRATQuery" ).ToElement();
      TiXmlElement* answer = h.FirstChild( "AnswerKey" ).ToElement();
      string attr_class_str =
        (attr_class)
        ? string( attr_class )
        : "";
      bool attr_class_good =
        (attr_class_str == "HistQueryTestCase") ||
        (attr_class_str == "IQRHistQueryTestCase" );
      if (attr_class_good && query && answer )
      {
        const char *attr_mtype = query->Attribute( "mtype" );
        string attr_mtype_str =
          (attr_mtype)
          ? string( attr_mtype )
          : "";
        if ( (attr_mtype_str == "hist" ) || ( attr_mtype_str == "histIQR") )
        {
          keep_this = true;
        }
      }

      if ( ! keep_this )
      {
        LOG_INFO( main_logger, "Ignoring TestCase at line " << xmlObject->Row() );
        ++tc_ignored_count;
        continue;
      }

      ++tc_kept_count;
      const char *attr_qid = query->Attribute( "id" );
      if ( ! attr_qid )
      {
        ostringstream oss;
        oss << "Malformed scenario: VIRATQuery without 'id' attribute at " << fn << " line " << query->Row()
            << "\n'" << *query << "'";

        throw runtime_error( oss.str() );
      }
      const char* attr_activity_list = answer->Attribute( "actType" );
      if ( ! attr_activity_list )
      {
        ostringstream oss;
        oss << "Malformed scenario: AnswerKey without 'actType' attribute at line " << answer->Row();
        throw runtime_error( oss.str() );
      }
      string activity_list_str( attr_activity_list );

      vector< string > parsed_activities;
      {
        boost::char_separator<char> delim( "," );
        boost::tokenizer< boost::char_separator< char > > act_tokens( activity_list_str, delim );
        parsed_activities.assign( act_tokens.begin(), act_tokens.end() );
      }

      vector< size_t > activity_list;
      for (size_t i=0; i<parsed_activities.size(); ++i)
      {
        activity_list.push_back( aries_interface::activity_to_index( parsed_activities[i] ));
      }

      pair< map< string, vector< size_t > >::iterator, bool > r =
        qid2activity_map.insert( make_pair( string( attr_qid ), activity_list)  );
      if ( ! r.second )
      {
        LOG_WARN( main_logger, "Duplicate query id " << attr_qid << " in scenario?" );
      }
    } // ...test case clause
  } // ... for all XML objects

  if (tc_kept_count + tc_ignored_count > 0)
  {
    LOG_INFO( main_logger, "Loading VIRAT scenario: kept " << tc_kept_count << " test cases; ignored " << tc_ignored_count );
  }
  return true;
}

} // ...virat_scenario_utilities

} // ...kwant
} // ...kwiver
