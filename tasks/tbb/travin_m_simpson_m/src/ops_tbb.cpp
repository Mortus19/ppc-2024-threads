// Copyright 2024 Travin Maksim
#include "tbb/travin_m_simpson_m/include/ops_tbb.hpp"

#include <thread>

using namespace std::chrono_literals;

double travin_tbb::Simpson(function func, double a, double b, double y) {
  return func(a, y) + 4 * func((a + b) / 2, y) + func(b, y);
}

double travin_tbb::SimpsonSeq(function func, int a, int b, int c, int d, int steps) {
  double result{};
  double sec1 = static_cast<double>(b - a) / steps;
  double sec2 = static_cast<double>(d - c) / steps;

  for (int i = 0; i < steps; i++) {
    double Ai1 = c + i * sec2;
    double Ai2 = c + (i + 1) * sec2;
    for (int j = 0; j < steps; j++) {
      double Bj1 = a + j * sec1;
      double Bj2 = a + (j + 1) * sec1;

      result += (sec1 * sec2 / 36) * (Simpson(func, Bj1, Bj2, Ai1) + 4 * Simpson(func, Bj1, Bj2, (Ai1 + Ai2) / 2) +
                                      Simpson(func, Bj1, Bj2, Ai2));
    }
  }

  return result;
}

double travin_tbb::SimpsonTbb(function func, int a, int b, int c, int d, int steps) {
  double result{};
  double sec1 = static_cast<double>(b - a) / steps;
  double sec2 = static_cast<double>(d - c) / steps;

  result = oneapi::tbb::parallel_reduce(
      oneapi::tbb::blocked_range<int>(0, steps), 0.0,
      [&](const tbb::blocked_range<int>& range, double tmp) {
        for (int i = range.begin(); i != range.end(); i++) {
          double Ai1 = c + i * sec2;
          double Ai2 = c + (i + 1) * sec2;
          for (int j = 0; j < steps; j++) {
            double Bj1 = a + j * sec1;
            double Bj2 = a + (j + 1) * sec1;

            tmp += (sec1 * sec2 / 36) * (Simpson(func, Bj1, Bj2, Ai1) + 4 * Simpson(func, Bj1, Bj2, (Ai1 + Ai2) / 2) +
                                         Simpson(func, Bj1, Bj2, Ai2));
          }
        }
        return tmp;
      },
      std::plus<>());

  return result;
}

double travin_tbb::LinFunc(double x, double y) { return 2 * x - y; };
double travin_tbb::TrigFunc(double x, double y) { return cos(x + y); };
double travin_tbb::MulFunc(double x, double y) { return x * y; };
double travin_tbb::ExpFunc(double x, double y) { return exp(x + y); }
double travin_tbb::RandFunc(double x, double y) { return x * x + y * y; }

bool travin_tbb::SimpsonTaskSeq::pre_processing() {
  internal_order_test();
  auto* data = reinterpret_cast<int*>(taskData->inputs[0]);
  a = data[0];
  b = data[1];
  c = data[2];
  d = data[3];
  n = data[4];

  res = 0.0;
  return true;
}

bool travin_tbb::SimpsonTaskSeq::validation() {
  internal_order_test();
  return taskData->inputs_count[0] == 5 && taskData->outputs_count[0] == 1;
}

bool travin_tbb::SimpsonTaskSeq::run() {
  internal_order_test();

  res = SimpsonSeq(func, a, b, c, d, n);
  return true;
}

bool travin_tbb::SimpsonTaskSeq::post_processing() {
  internal_order_test();
  reinterpret_cast<double*>(taskData->outputs[0])[0] = res;
  return true;
}

bool travin_tbb::SimpsonTaskTbb::pre_processing() {
  internal_order_test();
  auto* data = reinterpret_cast<int*>(taskData->inputs[0]);
  a = data[0];
  b = data[1];
  c = data[2];
  d = data[3];
  n = data[4];

  res = 0.0;
  return true;
}

bool travin_tbb::SimpsonTaskTbb::validation() {
  internal_order_test();
  return taskData->inputs_count[0] == 5 && taskData->outputs_count[0] == 1;
}

bool travin_tbb::SimpsonTaskTbb::run() {
  internal_order_test();

  res = SimpsonTbb(func, a, b, c, d, n);
  return true;
}

bool travin_tbb::SimpsonTaskTbb::post_processing() {
  internal_order_test();
  reinterpret_cast<double*>(taskData->outputs[0])[0] = res;
  return true;
}
