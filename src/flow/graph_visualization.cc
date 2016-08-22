#include "graph_visualization.h"
#include <ostream>
#include <sstream>
#include <fstream>
#include <stdexcept>
#include <typeinfo>
#include <map>
#include "graph.h"
#include "sync_node.h"
#include "async_node.h"
#include "sink_node.h"
#include "multiplex_node.h"

namespace mf { namespace flow {

using namespace std::literals;

namespace {
	template<typename T>
	std::string uid_(const T& object, const std::string& prefix) {
		return prefix + std::to_string(reinterpret_cast<std::uintptr_t>(&object));
		/*
		static std::map<const T*, std::string> ids = std::map<const T*, std::string>();
		auto it = ids.find(&object);
		if(it != ids.end()) {
			return it->second;
		} else {
			std::size_t index = ids.size();
			std::string id = prefix + std::to_string(index);
			auto it = ids.emplace(&object, id);
			return it.first->second;
		}*/
	}
}


graph_visualization::graph_visualization(const graph& gr, std::ostream& output) :
	output_(output),
	graph_(gr) { }


void graph_visualization::generate_node_dispatch_(const node& nd) {	
	if(typeid(nd) == typeid(sync_node))
		generate_processing_node_(static_cast<const processing_node&>(nd), false, false);
/*	else if(typeid(nd) == typeid(async_node))
		generate_processing_node_(static_cast<const processing_node&>(nd), true, false);*/
	else if(typeid(nd) == typeid(sink_node))
		generate_processing_node_(static_cast<const processing_node&>(nd), false, true);
	else if(typeid(nd) == typeid(multiplex_node))
		generate_multiplex_node_(static_cast<const multiplex_node&>(nd));
	else
		throw std::logic_error("unknown node type for visualization");
}


void graph_visualization::generate_processing_node_(const processing_node& nd, bool async, bool sink) {
	std::string node_id = uid_(nd, "node");
	
	std::ostringstream html;
	html << R"(<TABLE BORDER="0" CELLBORDER="0" CELLSPACING="0" CELLPADDING="0">)";
	
	if(nd.inputs_count() > 0) {
		html << R"(<TR>)";
		html << R"(<TD WIDTH="20"></TD>)";
		for(std::ptrdiff_t i = 0; i < nd.inputs_count(); ++i) {
			const node_input& in = nd.input_at(i);
			std::string input_name = "in";
			std::string input_id = uid_(in, "in");
			html << R"(<TD BORDER="1" CELLPADDING="1" PORT=")" << input_id << R"(">)";
			html << R"(<FONT POINT-SIZE="10">)" << nd.input_at(i).name() << R"(</FONT>)";
			html << R"(</TD>)";
			html << R"(<TD WIDTH="20"></TD>)";
		}
		html << R"(</TR>)";
	}
	
	std::size_t colspan = 2*nd.inputs_count() + 1;
	html << R"(<TR>)";
	html << R"(<TD COLSPAN=")" << colspan << R"(" BORDER="1" STYLE="ROUNDED" CELLPADDING="4">)";
	html << nd.name();
	html << R"(<BR/><FONT POINT-SIZE="10">)";
	if(sink) html << "sink node";
	else if(async) html << "async node";
	else html << "sync node";
	html << R"(</FONT>)";
	html << R"(</TD>)";
	html << R"(</TR>)";
	
	if(! sink) {
		html << R"(<TR>)";
		html << R"(<TD COLSPAN=")" << colspan << R"(" BORDER="0">)";
		html << R"(<TABLE BORDER="0" CELLBORDER="0" CELLSPACING="0" CELLPADDING="0">)";
		html << R"(<TR>)";
		html << R"(<TD WIDTH="20"></TD>)";
		html << R"(<TD BORDER="1" CELLPADDING="3" PORT=")" << uid_(nd.output(), "out") << R"(">)";
		html << R"(<TABLE BORDER="0" CELLSPACING="2">)";
		html << R"(<TR CELLPADDING="1">)";
		for(std::ptrdiff_t i = 0; i < nd.output_channels_count(); ++i) {
			const std::string& chan_name = nd.output_channel_at(i).name();
			if(i > 0) html << R"(<TD WIDTH="5"></TD>)";
			html << R"(<TD BORDER="1" CELLPADDING="1">)";
			html << R"(<FONT POINT-SIZE="10">)" << chan_name << R"(</FONT>)";
			html << R"(</TD>)";
		}
		html << R"(</TR>)";
		html << R"(</TABLE>)";
		html << R"(</TD>)";
		html << R"(<TD WIDTH="20"></TD>)";
		html << R"(</TR>)";
		html << R"(</TABLE>)";
		html << R"(</TD>)";
		html << R"(</TR>)";
	}
	
	html << R"(</TABLE>)";
	
	
	output_ << '\t' << node_id << " [shape=plaintext label=<\n" << html.str() << "\n>];\n";
}


void graph_visualization::generate_multiplex_node_(const multiplex_node& nd) {
	std::string node_id = uid_(nd, "node");
	
	std::ostringstream html;
	html << R"(<TABLE BORDER="0" CELLBORDER="0" CELLSPACING="0" CELLPADDING="0">)";
	
	std::string input_id = uid_(nd.input(), "in");
	html << R"(<TR>)";
	html << R"(<TD WIDTH="20"></TD>)";
	html << R"(<TD BORDER="1" CELLPADDING="1" PORT=")" << input_id << R"(">)";
	html << R"(<FONT POINT-SIZE="10">)" << nd.input().name() << R"(</FONT>)";
	html << R"(</TD>)";
	html << R"(<TD WIDTH="20"></TD>)";
	html << R"(</TR>)";
	
	html << R"(<TR>)";
	html << R"(<TD COLSPAN="3" BORDER="1" STYLE="ROUNDED" CELLPADDING="4">)";
	html << nd.name();
	html << R"(</TD>)";
	html << R"(</TR>)";
	
	html << R"(<TR>)";
	html << R"(<TD COLSPAN="3" BORDER="0">)";
	html << R"(<TABLE BORDER="0" CELLBORDER="0" CELLSPACING="0" CELLPADDING="0">)";
	html << R"(<TR>)";
	html << R"(<TD WIDTH="20"></TD>)";
	
	for(std::ptrdiff_t i = 0; i < nd.outputs_count(); ++i) {
		std::string output_id = uid_(nd.output_at(i), "out");
		html << R"(<TD BORDER="1" CELLPADDING="3" PORT=")" << output_id << R"(">)";
		html << R"(<TABLE BORDER="0" CELLSPACING="2">)";
		html << R"(<TR CELLPADDING="1">)";
		html << R"(<TD BORDER="1" CELLPADDING="1">)";
		html << R"(<FONT POINT-SIZE="10">)" << nd.output_at(i).name() << R"(</FONT>)";
		html << R"(</TD>)";
		html << R"(</TR>)";
		html << R"(</TABLE>)";
		html << R"(</TD>)";
		html << R"(<TD WIDTH="20"></TD>)";
	}
	
	html << R"(</TR>)";
	html << R"(</TABLE>)";
	html << R"(</TD>)";
	html << R"(</TR>)";
	
	html << R"(</TABLE>)";
	
	
	output_ << '\t' << node_id << " [shape=plaintext label=<\n" << html.str() << "\n>];\n";
}


void graph_visualization::generate_node_input_connections_(const node& nd) {
	for(std::ptrdiff_t i = 0; i < nd.inputs_count(); ++i) {
		const node_input& in = nd.input_at(i);
		if(! in.is_connected()) continue;
		const node_output& out = in.connected_output();
		
		output_
			<< '\t' << uid_(out.this_node(), "node") << ':' << uid_(out, "out")
			<< " -> " << uid_(nd, "node") << ':' << uid_(in, "in") << ";\n";
	}
}


void graph_visualization::generate_ranks_() {
	output_ << "\t{rank=source;";
	for(std::ptrdiff_t i = 0; i < graph_.nodes_count(); ++i) {
		const node& nd = graph_.node_at(i);
		if(nd.inputs_count() == 0) output_ << ' ' << uid_(nd, "node");
	}
	output_ << "}\n";
	
	const node& nd = graph_.sink();
	output_ << "\t{rank=sink; " << uid_(nd, "node") << "}\n";
}


void graph_visualization::generate() {
	output_ << "digraph " << graph_id_ << "{\n";
	output_ << "\trankdir=TB\n";
	for(std::ptrdiff_t i = 0; i < graph_.nodes_count(); ++i)
		generate_node_dispatch_(graph_.node_at(i));
	for(std::ptrdiff_t i = 0; i < graph_.nodes_count(); ++i)
		generate_node_input_connections_(graph_.node_at(i));
		
	generate_ranks_();	
			
	output_ << '}' << std::flush;
}


void export_graph_visualization(const graph& gr, const std::string& filename) {
	std::ofstream fstr(filename);
	graph_visualization vis(gr, fstr);
	vis.generate();
}

}}
