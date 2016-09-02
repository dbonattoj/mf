#ifndef MF_FLOW_PROCESSING_TIMELINE_JSON_H_
#define MF_FLOW_PROCESSING_TIMELINE_JSON_H_

#include <iosfwd>

namespace mf { namespace flow {

class processing_timeline;

void export_processing_timeline_to_json(const processing_timeline&, std::ostream&);
processing_timeline import_processing_timeline_from_json(std::istream&);

}}

#endif
