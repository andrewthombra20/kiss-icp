// MIT License
//
// Copyright (c) 2022 Ignacio Vizzo, Tiziano Guadagnino, Benedikt Mersch, Cyrill
// Stachniss.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
#include "Preprocessing.hpp"

#include <tbb/parallel_for.h>

#include <Eigen/Core>
#include <algorithm>
#include <sophus/se3.hpp>
#include <vector>

namespace {
/// TODO(Nacho) Explain what is the very important meaning of this param
constexpr double mid_pose_timestamp{0.5};
}  // namespace

namespace kiss_icp {

std::vector<Eigen::Vector3d> Preprocess(const std::vector<Eigen::Vector3d> &frame,
                                        double max_range,
                                        double min_range) {
    std::vector<Eigen::Vector3d> inliers;
    std::copy_if(frame.cbegin(), frame.cend(), std::back_inserter(inliers), [&](const auto &pt) {
        const double norm = pt.norm();
        return norm < max_range && norm > min_range;
    });
    return inliers;
}

std::vector<Eigen::Vector3d> DeSkewScan(const std::vector<Eigen::Vector3d> &frame,
                                        const std::vector<double> &timestamps,
                                        const Sophus::SE3d &delta) {
    const auto delta_pose = delta.log();
    std::vector<Eigen::Vector3d> corrected_frame(frame.size());
    // TODO(All): This tbb execution is ignoring the max_n_threads config value
    tbb::parallel_for(size_t(0), frame.size(), [&](size_t i) {
        const auto motion = Sophus::SE3d::exp((timestamps[i] - mid_pose_timestamp) * delta_pose);
        corrected_frame[i] = motion * frame[i];
    });
    return corrected_frame;
}
}  // namespace kiss_icp
