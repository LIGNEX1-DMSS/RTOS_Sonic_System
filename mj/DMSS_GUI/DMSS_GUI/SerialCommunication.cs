using System;
using System.IO.Ports;
using System.Text;
using System.Windows;
using DMSS_GUI.ViewModel;
using System.Diagnostics;

namespace DMSS_GUI
{
    public class SerialCommunication
    {
        private SerialPort _serialPort;
        private MainViewModel _viewModel;
        private TaskCompletionSource<bool> _fireAckTcs;
        string portName = "COM3";
        int baudRate = 115200;

        public SerialCommunication(MainViewModel viewModel)
        {
            _viewModel = viewModel;

            //try
            //{
            //    ConnectToPort(portName);
            //}
            //catch
            //{
            //    // 포트 연결안되면 여기서 다이어로그 띄울 예정
            //}
        }

        private void OnDataReceived(object sender, SerialDataReceivedEventArgs e)
        {
            try
            {
                string line = _serialPort.ReadLine().Trim(); // ex: "A:100,B:90,D:10"
                Debug.WriteLine($"Seiral Receive : {line}");
                App.Current.Dispatcher.Invoke(() =>
                {
                    if(line.StartsWith("A:"))
                    {
                        string[] parts = line.Trim().Split(',');

                        int a = int.Parse(parts[0].Split(':')[1]);
                        int b = int.Parse(parts[1].Split(':')[1]);
                        int d = int.Parse(parts[2].Split(':')[1]);

                        _viewModel.ReceiverA.SignalStrength = a;
                        _viewModel.ReceiverB.SignalStrength = b;
                        _viewModel.Diff = d;
                        _viewModel.UpdateReceiverColors();
                    }
                    else if(line.StartsWith("ACK_FIRE"))
                    {
                        //TrySetResult vs. SetResult 차이 정리하기
                        _fireAckTcs?.TrySetResult(true);  //FireAsync 대기 해제
                        _viewModel.AddLog("[RX] 발사 완료 확인됨");
                    }
                    else if (line.StartsWith("NACK_FIRE"))
                    {
                        _fireAckTcs?.TrySetResult(false);
                        _viewModel.AddLog("[RX] 발사 실패 응답");
                    }
                });
            }
            catch
            {
                //Todo : 에러 처리
                MessageBox.Show("Port Not connected");
            }        
        }

        public bool ConnectToPort(string portName)
        {
            try
            {
                _serialPort = new SerialPort
                {
                    PortName = portName,
                    BaudRate = 115200,
                    DataBits = 8,
                    Parity = Parity.None,
                    StopBits = StopBits.One,
                    Handshake = Handshake.None,
                    Encoding = Encoding.ASCII // 인코딩도 명시적으로 지정하면 안전함
                };

                _serialPort.DataReceived += OnDataReceived;
                _serialPort.Open();

                _viewModel.AddLog($"[System] 포트 {portName} 연결 완료");
                return true;
            }
            catch (Exception ex)
            {
                _viewModel.AddLog($"[Error] 포트 {portName} 연결 실패: {ex.Message}");
                return false;
            }
        }


        public async Task<bool> SendFireCommandAsync(int timeoutMilliseconds = 5000)
        {
            if (_serialPort == null || !_serialPort.IsOpen)
            {
                _viewModel.AddLog("[TX] Serial port not open.");
                return false;
            }

            _fireAckTcs = new TaskCompletionSource<bool>(); // 응답 대기용 Task

            // 1. 대상 선택 로직
            string target;

            if (_viewModel.FireMode == MainViewModel.fMode.Auto)
            {
                // 자동 선택 모드: 수신 강도 비교
                if (_viewModel.Diff > 0)
                    target = "A";
                else if (_viewModel.Diff < 0)
                    target = "B";
                else
                    target = "N"; // 동일한 강도일 경우
            }
            else
            {
                // 사용자 수동 선택
                target = _viewModel.SelectedTarget;
                if (string.IsNullOrEmpty(target))
                {
                    _viewModel.AddLog("[TX] 수동 발사 대상이 선택되지 않았습니다.");
                    return false;
                }
            }
            
            // 2. 명령 생성 및 전송
            string command = $"{target}";
            try
            {
                _serialPort.WriteLine(command); // \r\n 자동 포함
                _viewModel.AddLog($"[TX] {command}");
            }
            catch (Exception ex)
            {
                _viewModel.AddLog($"[TX Error] {ex.Message}");
                return false;
            }

            // Time Out Setting
            var timeoutTask = Task.Delay(timeoutMilliseconds);
            var completedTask = await Task.WhenAny(_fireAckTcs.Task, timeoutTask);

            if(completedTask == _fireAckTcs.Task)
            {
                return _fireAckTcs.Task.Result;
            } 
            else
            {
                _viewModel.AddLog("[RX TImeout] 발사 응답이 없습니다");
                return false;
            }
            // setting finish
        }

        public void Close() => _serialPort?.Close();
    }
}
