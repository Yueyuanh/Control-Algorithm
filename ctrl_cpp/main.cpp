#include "cpython.hpp"
#include "ctrl.h"
using namespace cpython; // import python



class FirstOrderSystem {
private:
    double K, tau;
    double state;
public:
    FirstOrderSystem(double K, double tau) {
        this->K = K;
        this->tau = tau;
        this->state = 0;
    }
    double update(double control, double dt) {
        state += (K * control - state) * (1 - std::exp(-dt / tau));
        return state;
    }
};



int main() {
    // ���ÿ�����
    double Kp = 5.0;
    double Ki = 1.5;
    double Kd = 0.2;
    ctrl::PIDController pid(Kp, Ki, Kd);

    // ���ù���ϵͳ
    double K = 1;
    double tau = 1.2;
    FirstOrderSystem sys(K, tau);

    // ���÷������
    double dt = 0.01;
    double tf = 20;
    int n_steps = static_cast<int>(tf / dt);

    // ���ù켣�ź�
    double t = 0;
    vector<double> setpoint;
    for (auto i : range(n_steps)) {
        setpoint.emplace_back(std::cos(t));
        t += dt;
    }

    // ����
    vector<double> state(n_steps, 0);
    vector<double> control(n_steps, 0);
    double u0 = 0.0;
    for (auto i : range(len(state))) {
        control[i] = u0;
        state[i] = sys.update(u0, dt);
        u0 = pid(setpoint[i], state[i], dt);
    }

    // ���
    for (auto i : range(len(state))) {
        print("Time =", i * dt, "|", "Setpoint =", setpoint[i], "|", "State =", state[i], "|", "Control =", control[i]);
    }

    return 0;
}