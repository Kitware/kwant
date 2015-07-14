/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "Security.h"
#include <vbl/vbl_smart_ptr.txx>
VBL_SMART_PTR_INSTANTIATE(STANAG_4676::Security);

namespace STANAG_4676
{

/// \brief Initialize
Security
::Security(const ClassificationLevel& level, const std::string& policy)
  : _classification(level), _policyName(policy)
{
}

/// \brief Provides security classification level.
///
/// A character string used to identify the security information applicable for
/// the item or class being reported.  Provides the \em highest Security
/// designation of the Track Message, Track and/or Track Item. The attribute
/// can be used to facilitate the exchange of information through Information
/// Exchange Gateways and Guards.  The message security level shall not be
/// lower than the highest security label of Track or Track Item Classes.
STANAG_4676_IMPLEMENT_POD_ATTRIBUTE(
  Security,
  ClassificationLevel,
  Classification,
  _classification)

/// \brief Provides name of National policy under which security information
///        is specified.
///
/// A character string used to identify the nation or enterprise (eg. NATO)
/// responsible for creating, maintaining and implementing the security policy
/// to be applied to the information, in accordance with
/// [EAPC(AC/322-SC/5)N(2006)0008]. Value for National policy shall be 3-letter
/// country codes in compliance with STANAG 1059.  Value for Coalitions or
/// other entities (e.g. "NATO" or "ISAF" or "EAPC") shall be expressed as a
/// 4-character alphanumeric string.
///
/// Examples -  NATO/EAPC, Security Policy: USA
STANAG_4676_IMPLEMENT_OBJECT_ATTRIBUTE(
  Security,
  std::string,
  PolicyName,
  _policyName)

/// \brief Provides control system under which reported data is protected.
///
/// A character string used to identify the applicable Control System in
/// accordance with National policy specified in securityPolicyName attribute.
/// Examples include but are not limited to: ATOMAL, CRYPTO, SIOP, SIOP ESI,
/// TK.
STANAG_4676_IMPLEMENT_OBJECT_ATTRIBUTE(
  Security,
  std::vector<std::string>,
  ControlSystems,
  _controlSystems)

/// \brief Convenience method to add a control system.
void
Security
::addControlSystem(const std::string& system)
{
  this->_controlSystems.push_back(system);
}

/// \brief Provides dissemination instructions for reported data.
///
/// A character string used to identify Dissemination limitations in accordance
/// with National policy specified in securityPolicyName attribute.  Examples
/// include but are not limited to: EXCLUSIVE, INTELLIGENCE, LOGISTICS,
/// OPERATIONS, FOUO, RSEN, NOFORN, etc.
STANAG_4676_IMPLEMENT_OBJECT_ATTRIBUTE(
  Security,
  std::vector<std::string>,
  Disseminations,
  _disseminations)

/// \brief Convenience method to add a dissemination.
void
Security
::addDissemination(const std::string& d)
{
  this->_disseminations.push_back(d);
}

/// \brief Provides releasability restrictions for the reported data.
///
/// A character string used to identify Releasability specifications. Value(s)
/// for Nation(s) shall be 3-letter country code(s) in compliance with
/// STANAG 1059.  Allowed values for coalitions are: NATO, EU, PFP, MD, ISAF,
/// 4 EYES, 5 EYES, and 9 EYES.
STANAG_4676_IMPLEMENT_OBJECT_ATTRIBUTE(
  Security,
  std::vector<std::string>,
  Releasability,
  _releasability)

/// \brief Convenience method to add a releasability.
void
Security
::addReleasability(const std::string& r)
{
  this->_releasability.push_back(r);
}

TiXmlElement*
Security
::toXML(const std::string& name) const
{
  TiXmlElement* e = new TiXmlElement(name);
  e->SetAttribute("xsi:type", "Security");
  e->LinkEndChild(XMLIOBase::toXML("securityClassification", toString(this->_classification)));
  e->LinkEndChild(XMLIOBase::toXML("securityPolicyName", this->_policyName));
  for (size_t i = 0; i < this->_controlSystems.size(); ++i)
  {
    e->LinkEndChild(XMLIOBase::toXML("securityControlSystem", this->_controlSystems[i]));
  }
  for (size_t i = 0; i < this->_disseminations.size(); ++i)
  {
    e->LinkEndChild(XMLIOBase::toXML("securityDissemination", this->_disseminations[i]));
  }
  for (size_t i = 0; i < this->_releasability.size(); ++i)
  {
    e->LinkEndChild(XMLIOBase::toXML("securityReleasability", this->_releasability[i]));
  }
  return e;
}

Security
::Security(const TiXmlElement* elem)
{
  XMLIOBase::fromXML(elem, "securityClassification", this->_classification);
  XMLIOBase::fromXML(elem, "securityPolicyName", this->_policyName);
  XMLIOBase::fromXML(elem, "securityControlSystem", this->_controlSystems);
  XMLIOBase::fromXML(elem, "securityDissemination", this->_disseminations);
  XMLIOBase::fromXML(elem, "securityReleasability", this->_releasability);
}

STANAG_4676_FROMXML(Security)

} // End STANAG_4676 namespace
