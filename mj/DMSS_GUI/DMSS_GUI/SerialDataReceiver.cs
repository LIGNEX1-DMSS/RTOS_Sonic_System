using System;
using System.IO.Ports;
using DMSS_GUI.ViewModel;

namespace DMSS_GUI
{
    public class SerialDataReceiver
    {
        private SerialPort _serialPort;
        private MainViewModel _viewModel;

        public SerialDataReceiver(MainViewModel viewModel)
        {
            _viewModel = viewModel;
            _serialPort = new SerialPort("COM3", 115200);
            _serialPort.DataReceived += OnDataReceived;
            _serialPort.Open();
        }

        private void OnDataReceived(object sender, SerialDataReceivedEventArgs e)
        {
            try
            {
                string line = _serialPort.ReadLine(); // ex: "A:100,B:90,D:10"
                string[] parts = line.Trim().Split(',');

                int a = int.Parse(parts[0].Split(':')[1]);
                int b = int.Parse(parts[1].Split(':')[1]);
                int d = int.Parse(parts[2].Split(':')[1]);

                App.Current.Dispatcher.Invoke(() =>
                {
                    _viewModel.ReceiverA.SignalStrength = a;
                    _viewModel.ReceiverB.SignalStrength = b;
                    _viewModel.Diff = d;
                    _viewModel.UpdateReceiverColors();
                });
            }
            catch
            {
                //Todo : 에러 처리
            }        
        }

        public void Close() => _serialPort?.Close();
    }
}
