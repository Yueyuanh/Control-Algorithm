#include "pid.h"


// һά�ź�
PIDController::PIDController(
    double Kp,
    double Ki,
    double Kd,
    const pair<double, double>& control_limits,
    const pair<double, double>& integral_limits,
    double feedforward_gain
): 
    __Kp(1, Kp),
    __Ki(1, Ki), 
    __Kd(1, Kd), 
    __control_limits(1, control_limits),
    __integral_limits(1, integral_limits),
    __feedforward_gain(1, feedforward_gain),
    __last_error(1, 0.0), 
    __integral(1, 0.0) 
{}

double PIDController::call(double setpoint, double feedback, double dt, double expected_output) {
    vector<double> setpoints(1, setpoint); // vector(dim, init)
    vector<double> feedbacks(1, feedback);
    vector<double> expected_outputs;
    if (expected_output != 0.0) {
        expected_outputs.push_back(expected_output);
    }
    vector<double> outputs = this->call(setpoints, feedbacks, dt, expected_outputs);
    return outputs[0];
}

double PIDController::operator()(double setpoint, double feedback, double dt, double expected_output) {
    return this->call(setpoint, feedback, dt, expected_output);
}



// ��ά�ź�
PIDController::PIDController(
    const vector<double>& Kp,
    const vector<double>& Ki,
    const vector<double>& Kd,
    const vector<pair<double, double>>& controls_limits,
    const vector<pair<double, double>>& integrals_limits,
  
    const vector<double>& feedforward_gain
):
    __Kp(Kp), 
    __Ki(Ki), 
    __Kd(Kd), 
    __control_limits(controls_limits),
    __integral_limits(integrals_limits),
    __feedforward_gain(feedforward_gain),
    __last_error(Kp.size(), 0.0),
    __integral(Kp.size(), 0.0)
{}

vector<double> PIDController::call(const vector<double>& setpoint, const vector<double>& feedback, double dt, const vector<double>& expected_output) {
    // ���ά��
    if (setpoint.size() != feedback.size() || setpoint.size() != __Kp.size()) {
        throw std::invalid_argument("Dimension mismatch between setpoint, feedback, and parameters.");
    }

    vector<double> proportional(this->__Kp.size(), 0.0); // ������
    vector<double> integral_term(this->__Ki.size(), 0.0); // ������
    vector<double> derivative(this->__Kd.size(), 0.0); // ΢����
    vector<double> feedforward(this->__feedforward_gain.size(), 0.0); // ǰ����
    vector<double> control(this->__Kp.size(), 0.0); // ������

    for (int i = 0; i < this->__Kp.size(); i++) {
        double error = setpoint[i] - feedback[i];

        proportional[i] = this->__Kp[i] * error;

        // Anti-windup (feedback limiting)
        if (!this->__integral_limits.empty() && i < this->__integral_limits.size()) {
            this->__integral[i] = clip(this->__integral[i] + error * dt, this->__integral_limits[i].first, this->__integral_limits[i].second);
        }
        else {
            this->__integral[i] += error * dt;
        }

        integral_term[i] = this->__Ki[i] * this->__integral[i];
        derivative[i] = this->__Kd[i] * (error - this->__last_error[i]) / dt;

        // Feedforward control
        if (!this->__feedforward_gain.empty() && i < this->__feedforward_gain.size() && !expected_output.empty() && i < expected_output.size()) {
            feedforward[i] = this->__feedforward_gain[i] * (expected_output[i] - feedback[i]); // ǰ������ (�������y - �����ź�y)
        }

        control[i] = proportional[i] + integral_term[i] + derivative[i] + feedforward[i];

        // Output limiting
        if (!this->__control_limits.empty() && i < this->__control_limits.size()) {
            control[i] = clip(control[i], this->__control_limits[i].first, this->__control_limits[i].second);
        }

        this->__last_error[i] = error;
    }

    return control;

}

vector<double> PIDController::operator()(const vector<double>& setpoint, const vector<double>& feedback, double dt, const vector<double>& expected_output) {
    return this->call(setpoint, feedback, dt);
}


