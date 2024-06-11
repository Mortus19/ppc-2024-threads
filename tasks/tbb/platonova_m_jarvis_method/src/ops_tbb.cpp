// Copyright 2024 Platonova Mariya
#include "tbb/platonova_m_jarvis_method/include/ops_tbb.hpp"

#include <tbb/tbb.h>

#include <iostream>
#include <numeric>
#include <random>
#include <string>
#include <thread>
#include <vector>

using namespace std::chrono_literals;

// std::vector<Point> Jarvis(const std::vector<Point>& Points) {
//     if (Points.size() < 3) return Points;
//     Point p0 = *std::min_element(Points.begin(), Points.end(), [](const Point& a, const Point& b) { return a.x < b.x || (a.x == b.x && a.y < b.y); });
//     std::vector<Point> convexHull = {p0};
//     Point prevPoint = p0, nextPoint;

//     do {
//         nextPoint = Points[0];
//         tbb::parallel_for(
//             tbb::blocked_range<size_t>(0, Points.size()),
//             [&Points, &prevPoint, &nextPoint](const tbb::blocked_range<size_t>& r) {
//                 for (size_t i = r.begin(); i != r.end(); ++i) {
//                     const auto& point = Points[i];
//                     if (point == prevPoint) continue;
//                     double crossProduct = (point.y - prevPoint.y) * (nextPoint.x - prevPoint.x) - (point.x - prevPoint.x) * (nextPoint.y - prevPoint.y);
//                     double distPrevPoint = pow(point.x - prevPoint.x, 2) + pow(point.y - prevPoint.y, 2);
//                     double distNextPoint = pow(nextPoint.x - prevPoint.x, 2) + pow(nextPoint.y - prevPoint.y, 2);
//                     if (crossProduct > 0 || (crossProduct == 0 && distPrevPoint > distNextPoint)) {
//                         nextPoint = point;
//                     }
//                 }
//             }
//         );
//         convexHull.push_back(nextPoint);
//         prevPoint = nextPoint;
//     } while (nextPoint != p0);

//     return convexHull;
// }

std::vector<Point> Jarvis(const std::vector<Point>& Points, int num_threads) {
  if (Points.size() < 3) return Points;

  tbb::global_control control(tbb::global_control::max_allowed_parallelism, num_threads);

  Point p0 = *std::min_element(Points.begin(), Points.end(), [](const Point& a, const Point& b) { return a.x < b.x || (a.x == b.x && a.y < b.y); });
  std::vector<Point> convexHull = {p0};

  do {
    Point nextPoint = Points[0];
    double maxAngle = std::numeric_limits<double>::lowest();

    tbb::parallel_for_each(Points.begin(), Points.end(), [&Points, &convexHull, &nextPoint, &maxAngle](const Point& point) {
      if (point == convexHull.back()) return;
      double angle = std::atan2(point.y - convexHull.back().y, point.x - convexHull.back().x);
      if (angle > maxAngle) {
        maxAngle = angle;
        nextPoint = point;
      }
    });

    if (nextPoint == p0) break;

    convexHull.push_back(nextPoint);
  } while (convexHull.back() != p0);

  return convexHull;
}

bool JarvisTBB::pre_processing() {
    internal_order_test();
    points.resize(taskData->inputs_count[0]);
    auto* tmp_ptr = reinterpret_cast<Point*>(taskData->inputs[0]);
    std::copy_n(tmp_ptr, taskData->inputs_count[0], points.begin());
    return true;
}

bool JarvisTBB::validation() {
    internal_order_test();
    if (taskData->inputs_count[0] == 0) {
        return false;
    }
    std::sort(points.begin(), points.end());
    return std::unique(points.begin(), points.end()) == points.end();
}

bool JarvisTBB::run() {
    internal_order_test();
    resPoints = Jarvis(points,10);
    return true;
}

bool JarvisTBB::post_processing() {
    internal_order_test();
    auto* output_ptr = reinterpret_cast<Point*>(taskData->outputs[0]);
    std::copy_n(resPoints.begin(), resPoints.size(), output_ptr);
    return true;
}
