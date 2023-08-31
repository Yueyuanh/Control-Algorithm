#include "pid.h"


/* init pid */
PIDController::PIDController(
    double Kp,
    double Ki,
    double Kd,
    const pair<double, double>& control_limits,
    double Kf,
    double Kaw,
    const pair<double, double>& error_clip
): 
    __Kp(1, Kp),
    __Ki(1, Ki), 
    __Kd(1, Kd), 
    __Kf(1, Kf),
    __Kaw(1, Kaw),
    __control_limits(1, control_limits),
    __error_clip(1, error_clip),
    __last_error(1, 0.0), 
    __integral_error(1, 0.0),
    __u(1, 0.0)
{}

PIDController::PIDController(
    const vector<double>& Kp,
    const vector<double>& Ki,
    const vector<double>& Kd,
    const vector<pair<double, double>>& controls_limits,
    const vector<double>& Kf,
    const vector<double>& Kaw,
    const vector<pair<double, double>>& error_clip
):
    __Kp(Kp), 
    __Ki(Ki), 
    __Kd(Kd), 
    __last_error(Kp.size(), 0.0),
    __integral_error(Kp.size(), 0.0),
    __u(Kp.size(), 0.0)
{
    if (Kaw.empty()) {
        __Kaw = vector<double>(Kp.size(), 0.2);
    }
    else {
        __Kaw = Kaw;
    }
    if (Kf.empty()) {
        __Kf = vector<double>(Kp.size(), 0.0);
    }
    else {
        __Kf = Kf;
    }
    if (controls_limits.empty()) {
        __control_limits = vector<pair<double, double>>(Kp.size(), { -Infinity, Infinity });
    }
    else {
        __control_limits = controls_limits;
    }
    if (error_clip.empty()) {
        __error_clip = vector<pair<double, double>>(Kp.size(), { -Infinity, Infinity });
    }
    else {
        __error_clip = error_clip;
    }
    if (Kp.size() != Ki.size() || Kp.size() != Kd.size()) {
        throw std::invalid_argument("Dimensional inconsistency!");
    }
}





/* call pid */
double PIDController::call(double setpoint, double feedback, double dt, double expected_output, int anti_windup_method) {
    vector<double> expected_outputs;
    if (expected_output != 0.0) {
        expected_outputs.emplace_back(expected_output);
    }
    return this->call(vector<double>{setpoint}, vector<double>{feedback}, dt, expected_outputs, anti_windup_method)[0];
}
double PIDController::operator()(double setpoint, double feedback, double dt, double expected_output, int anti_windup_method) {
    return this->call(setpoint, feedback, dt, expected_output, anti_windup_method);
}


vector<double> PIDController::call(
    const vector<double>& setpoint, const vector<double>& feedback, double dt, 
    const vector<double>& expected_output, int anti_windup_method) {
    // ���ά��
    if (setpoint.size() != feedback.size() || setpoint.size() != __Kp.size()) {
        throw std::invalid_argument("Dimension mismatch between setpoint, feedback, and parameters.");
    }
    if (!expected_output.empty() && expected_output.size() != __Kp.size()) {
        throw std::invalid_argument("Dimension mismatch between expected_output, feedback, and parameters.");
    }

    vector<double> proportional(__Kp.size(), 0.0); // ������ Kp * e
    vector<double> integral(__Ki.size(), 0.0); // ������ Ki * ie
    vector<double> differential(__Kd.size(), 0.0); // ΢���� Kd * de
    vector<double> feedforward(__Kf.size(), 0.0); // ǰ����
    double error, u0;

    for (size_t i = 0; i < __Kp.size(); i++) {
        error = setpoint[i] - feedback[i];

        // PID control
        proportional[i] = __Kp[i] * error;
        differential[i] = __Kd[i] * (error - __last_error[i]) / dt;
        integral[i] = this->_AntiWindup(__Ki[i], __integral_error[i], error, __error_clip[i], __u[i], __control_limits[i], __Kaw[i], dt, anti_windup_method);

        // Feedforward control
        if (!expected_output.empty() && i < expected_output.size()) {
            feedforward[i] = __Kf[i] * (expected_output[i] - feedback[i]); // ǰ������ (�������y - �����ź�y)
        }

        // Control
        u0 = proportional[i] + integral[i] + differential[i] + feedforward[i];
        if (!__control_limits.empty() && i < __control_limits.size()) { //�Լ����õĲ�һ����u�ȳ�, û��init�н���u_limά�ȼ��
            u0 = clip(u0, __control_limits[i].first, __control_limits[i].second); 
        }
        __u[i] = u0;

        // Remember
        __last_error[i] = error; //__integral_error�Ѿ��ں����и�����
    }
    return __u;
}

vector<double> PIDController::operator()(
    const vector<double>& setpoint, const vector<double>& feedback, double dt, 
    const vector<double>& expected_output, int anti_windup_method) {
    return this->call(setpoint, feedback, dt, expected_output, anti_windup_method);
}



/* class method */
double PIDController::_AntiWindup(double Ki, double& integral_error, double error, pair<double, double> error_lim,
    double u, pair<double, double> u_lim, double Kaw, double dt, int method) {
    /* integral_error��������, integral�ǻ�����, ���ػ�����, ͬʱ������������ */
    method = method % 2;
    // �Ƿ���������
    double beta = 0.0;
    if (error >= error_lim.first && error <= error_lim.second) {
        beta = 1.0; // �����޲����������
    }
    // ����0 - ���������ַ���ȷ���Ƿ��ۼ����
    if (method == 0) {
        double gamma = 0.0;
        if (u > u_lim.second) {
            if (error < 0) {
                gamma = 1.0; // ���Ƴ������ۼӸ�ƫ��(gamma=1)�����޲��ۼ�(beta=0)
            }
        }
        else if (u < u_lim.first) {
            if (error > 0) {
                gamma = 1.0; // ���Ƴ������ۼ���ƫ��(gamma=1)�����޲��ۼ�(beta=0)
            }
        }
        else {
            gamma = 1.0; // ���Ʋ����������ۼ�ƫ��(gamma=1)�����޲��ۼ�(beta=0)
        }
        integral_error += beta * gamma * error * dt;
    }
    // ����1 - �������ƿ������㷨
    else {
        double antiWindupError = clip(u, u_lim.first, u_lim.second) - u;
        integral_error += error * dt + Kaw * antiWindupError; // �ۼ������ϸ�����ƫ��ķ�����
    }
    
    return beta * Ki * integral_error; // �Ƿ���������
}