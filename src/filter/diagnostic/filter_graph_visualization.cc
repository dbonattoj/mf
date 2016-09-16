/*
Author : Tim Lenertz
Date : May 2016

Copyright (c) 2016, Université libre de Bruxelles

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated
documentation files to deal in the Software without restriction, including the rights to use, copy, modify, merge,
publish the Software, and to permit persons to whom the Software is furnished to do so, subject to the following
conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the
Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include "filter_graph_visualization.h"
#include "../filter.h"
#include "../filter_graph.h"
#include "../filter_parameter.h"
#include <ostream>
#include <fstream>

namespace mf { namespace flow {

filter_graph_visualization::filter_graph_visualization(const filter_graph& gr, std::ostream& output) :
	output_(output),
	graph_(gr) { }


void filter_graph_visualization::generate_filter_(const filter& filt) {
	std::string node_id = uid_pool_.uid(filt, "filter");
		
	std::ostringstream html;
	html << R"(<TABLE BORDER="0" CELLBORDER="0" CELLSPACING="0" CELLPADDING="0">)";
	
	if(filt.inputs_count() > 0) {
		html << R"(<TR>)";
		html << R"(<TD WIDTH="20"></TD>)";
		for(std::ptrdiff_t i = 0; i < filt.inputs_count(); ++i) {
			const filter_input_base& in = filt.input_at(i);
			std::string input_name = "in";
			if(! filt.input_at(i).name().empty()) input_name = filt.input_at(i).name();
			
			std::string input_id = uid_pool_.uid(in, "in");
			html << R"(<TD BORDER="1" CELLPADDING="1" PORT=")" << input_id << R"(">)";
			html << R"(<FONT POINT-SIZE="10">)" << input_name << R"(</FONT>)";
			html << R"(</TD>)";
			html << R"(<TD WIDTH="20"></TD>)";
		}
		html << R"(</TR>)";
	}
	
	std::size_t colspan = 2*filt.inputs_count() + 1;
	html << R"(<TR>)";
	html << R"(<TD COLSPAN=")" << colspan << R"(" BORDER="1" CELLPADDING="4">)";
	html << filt.name();
	html << R"(<BR/><FONT POINT-SIZE="10">&nbsp;)";
	if(filt.is_asynchonous()) html << "prefetch = " << filt.prefetch_duration();
	html << R"(</FONT>)";
	

	if(with_parameters_ && filt.parameters_count() > 0) {
		html << R"(<BR/><BR/>)";
		for(std::ptrdiff_t i = 0; i < filt.parameters_count(); ++i) {
			const filter_parameter_base& param = filt.parameter_at(i);
			if(param.kind() == filter_parameter_base::reference) {
				std::string type;
				if(param.is_input_reference() && param.is_sent_reference()) type = "in+sent";
				else if(param.is_input_reference()) type = "in";
				else if(param.is_sent_reference()) type = "sent";

				std::string name;
				if(! param.name().empty()) name = param.name() + " ";
				name += "&#x2192; " + param.referenced_parameter().this_filter().name() + "." + param.referenced_parameter().name();

				html << R"(&#x25A1;<FONT POINT-SIZE="10"><I>)" << type << R"(</I>: )" << name << R"(</FONT><BR/>)";

			} else {
				std::string type;
				if(param.kind() == filter_parameter_base::deterministic) type = "det";
				else if(param.kind() == filter_parameter_base::dynamic) type = "dyn";


				html << R"(&#x25A0;<FONT POINT-SIZE="10"><I>)" << type << R"(</I>: )" << filt.parameter_at(i).name() << R"(</FONT><BR/>)";

			}
		}
	}
		
	html << R"(</TD>)";
	html << R"(</TR>)";
	
	if(filt.outputs_count() > 0) {
		html << R"(<TR>)";
		html << R"(<TD COLSPAN=")" << colspan << R"(" BORDER="0">)";
		html << R"(<TABLE BORDER="0" CELLBORDER="0" CELLSPACING="0" CELLPADDING="0">)";
		html << R"(<TR>)";
		
		html << R"(<TD WIDTH="20"></TD>)";
		
		for(std::ptrdiff_t i = 0; i < filt.outputs_count(); ++i) {
			std::string output_id = uid_pool_.uid(filt.output_at(i), "out");
			std::string output_name = "out";
			if(! filt.output_at(i).name().empty()) output_name = filt.output_at(i).name();


			html << R"(<TD BORDER="1" CELLPADDING="1" PORT=")" << output_id << R"(">)";
			html << R"(<FONT POINT-SIZE="10">)" << output_name << R"(</FONT>)";
			html << R"(</TD>)";
			html << R"(<TD WIDTH="20"></TD>)";
		}
		
		html << R"(</TR>)";
		html << R"(</TABLE>)";
		html << R"(</TD>)";
		html << R"(</TR>)";
	}
	
	html << R"(</TABLE>)";
	
	output_ << '\t' << node_id << " [shape=plaintext label=<\n" << html.str() << "\n>];\n";
}


void filter_graph_visualization::generate_filter_input_connections_(const filter& filt) {
	for(std::ptrdiff_t i = 0; i < filt.inputs_count(); ++i) {
		const filter_input_base& in = filt.input_at(i);
		if(! in.is_connected()) continue;
		const filter_output_base& connected_out = in.connected_output();
		const filter& connected_filt = in.connected_filter();
		
		std::string label;
		/*
		time_unit p = in.past_window_duration();
		time_unit f = in.future_window_duration();
		if(p > 0 && f > 0) label = "[-"s + std::to_string(p) + ", +" + std::to_string(f) + "]     ";
		else if(p > 0) label = "[-"s + std::to_string(p) + "]  ";
		else if(f > 0) label = "[+" + std::to_string(f) + "]  ";
		*/
				
		output_
			<< '\t' << uid_pool_.uid(connected_filt, "filter") << ':' << uid_pool_.uid(connected_out, "out")
			<< " -> " << uid_pool_.uid(filt, "filter") << ':' << uid_pool_.uid(in, "in") << " ["
			<< "arrowhead=\"normal\", "
			<< "headlabel=<" << label << ">, "
			<< "fontsize=10, "
			<< "labelangle=45, "
			<< "labeldistance=2.0, "
			<< "];\n";
	}
}


void filter_graph_visualization::generate_ranks_() {
	output_ << "\t{rank=source;";
	for(std::ptrdiff_t i = 0; i < graph_.filters_count(); ++i) {
		const filter& filt = graph_.filter_at(i);
		if(filt.inputs_count() == 0) output_ << ' ' << uid_pool_.uid(filt, "filter");
	}
	output_ << "}\n";
	
	output_ << "\t{rank=sink;";
	for(std::ptrdiff_t i = 0; i < graph_.filters_count(); ++i) {
		const filter& filt = graph_.filter_at(i);
		if(filt.outputs_count() == 0) output_ << ' ' << uid_pool_.uid(filt, "filter");
	}
	output_ << "}\n";
}

	
void filter_graph_visualization::generate() {
	output_ << "digraph " << graph_id_ << "{\n";
	output_ << "\trankdir=TB\n";

	for(std::ptrdiff_t i = 0; i < graph_.filters_count(); ++i) generate_filter_(graph_.filter_at(i));
	for(std::ptrdiff_t i = 0; i < graph_.filters_count(); ++i) generate_filter_input_connections_(graph_.filter_at(i));
	generate_ranks_();	
			
	output_ << "}\n" << std::flush;
}


///////////////


void export_filter_graph_visualization(const filter_graph& gr, const std::string& filename) {
	std::ofstream fstr(filename);
	filter_graph_visualization vis(gr, fstr);
	vis.generate();
}


}}
