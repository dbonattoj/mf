#ifndef MF_FLOW_FILTER_PARAMETER_H_
#define MF_FLOW_FILTER_PARAMETER_H_

namespace mf { namespace flow {

enum class parameter_kind {
	pulled,
	dynamic,
	deterministic
};

template<typename Value>
class filter_parameter {
public:
	using value_type = Value;

private:
	parameter_kind kind_;
};


template<typename Value>
class filter_external_parameter {
public:
	using value_type = Value;
	using parameter_type = filter_parameter<Value>;

private:
	parameter_type* referenced_parameter_;
};


}}

#endif

/*
VALUATION (for non-internal)
- pulled    ==> set 1 time by connected node in pre_process
            ==> pulled from connected successor, response generated with that value, inhibits prefetch

- dynamic   ==> set from outside (possib. multiple times), node uses latest value, can aggr. multiple updates

- animated  ==> function of time or frame index

- constant  ==> fixed before launch

- internal  ==> set by node itself, per processed frame

job.param(my_param) --> value to use
                                <-- internal: determined value

job.in_param(left_image_input.connected_filter().)







class left_filter {
	parameter<pose> left_pose;
}


class merge_filter {
	parameter<pose> virtual_pose;

	extern_parameter<pose> left_pose;

	preprocess() {
		job.pull_param(left_pose, pose1);
	}
	
	process() {
		print(job.param(left_pose));
	}
}



setup() {
	filter left_syn;
	filter merge;
	
	merge.left_pose
}









*/






