/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef VIDTK_LIBRARY_STANAG_4676_SECURITY_H_
#define VIDTK_LIBRARY_STANAG_4676_SECURITY_H_

#include <stanag_4676/XMLIOBase.h>
#include <stanag_4676/Enums.h>

namespace STANAG_4676
{

/// \brief Provides parameters pertinent to security, in accordance with
///        [EAPC(AC/322-SC/5)N(2006)0008]
class Security : public XMLIOBase
{
public:
  typedef vbl_smart_ptr<Security> sptr;

  Security(const ClassificationLevel& level, const std::string& policy);

  virtual ~Security(void) { }

  STANAG_4676_DECLARE_POD_ATTRIBUTE(ClassificationLevel, Classification)
  STANAG_4676_DECLARE_OBJECT_ATTRIBUTE(std::string, PolicyName)
  STANAG_4676_DECLARE_OBJECT_ATTRIBUTE(std::vector<std::string>, ControlSystems)
  STANAG_4676_DECLARE_OBJECT_ATTRIBUTE(std::vector<std::string>, Disseminations)
  STANAG_4676_DECLARE_OBJECT_ATTRIBUTE(std::vector<std::string>, Releasability)

  void addControlSystem(const std::string& system);
  void addDissemination(const std::string& dissemination);
  void addReleasability(const std::string& releasability);

  TiXmlElement* toXML(const std::string& name) const;

  static sptr fromXML(const TiXmlElement* elem, int* error_count = 0);

protected:
  Security(const TiXmlElement* elem);

private:
  ClassificationLevel _classification;
  std::string _policyName;
  std::vector<std::string> _controlSystems;
  std::vector<std::string> _disseminations;
  std::vector<std::string> _releasability;
};

} // End STANAG_4676 namespace

#endif
