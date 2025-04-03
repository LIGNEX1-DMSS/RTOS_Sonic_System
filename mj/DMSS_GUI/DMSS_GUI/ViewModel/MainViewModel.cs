using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Linq;
using System.Reflection.Metadata;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Input;
using System.Windows.Media;
using DMSS_GUI.Model;
using DMSS_GUI.command;
using System.Configuration;
using System.Collections.ObjectModel;
using System.Windows;
using System.IO.Ports;
using DMSS_GUI.Interface;

namespace DMSS_GUI.ViewModel
{
    public class MainViewModel : INotifyPropertyChanged
    {
        private ReceiverModel _receiverA;
        private ReceiverModel _receiverB;
        private Brush _receiverAColor = Brushes.LightGray;
        private Brush _receiverBColor = Brushes.LightGray;
        private int _diff;
        private string _systemStatus;
        private Brush _statusColor;
        private bool _isManualMode;
        private bool _targetASelected;
        private bool _targetBSelected;
        private bool _isFire;
        public readonly IDialogService _dialogService;
        public SerialCommunication Serial { get; private set; }

        public ReceiverModel ReceiverA
        {
            get => _receiverA;
            set { _receiverA = value; OnPropertyChanged(nameof(ReceiverA)); }
        }

        public ReceiverModel ReceiverB
        {
            get => _receiverB;
            set { _receiverB = value; OnPropertyChanged(nameof(ReceiverB)); }
        }
        public Brush ReceiverAColor
        {
            get => _receiverAColor;
            set { _receiverAColor = value; OnPropertyChanged(nameof(ReceiverAColor)); }
        }

        public Brush ReceiverBColor
        {
            get => _receiverBColor;
            set { _receiverBColor = value; OnPropertyChanged(nameof(ReceiverBColor)); }
        }

        public int Diff
        {
            get => _diff;
            set { _diff = value; OnPropertyChanged("diff"); }
        }
        public string SystemStatus
        {
            get => _systemStatus;
            set { _systemStatus = value; OnPropertyChanged(nameof(SystemStatus)); }
        }

        public Brush StatusColor
        {
            get => _statusColor;
            set { _statusColor = value; OnPropertyChanged(nameof(StatusColor)); }
        }

        public bool IsManualMode
        {
            get => _isManualMode;
            set { _isManualMode = value; OnPropertyChanged(nameof(IsManualMode)); }
        }

        public bool TargetASelected
        {
            get => _targetASelected;
            set { _targetASelected = value; OnPropertyChanged(nameof(TargetASelected)); }
        }

        public bool TargetBSelected
        {
            get => _targetBSelected;
            set { _targetBSelected = value; OnPropertyChanged(nameof(TargetBSelected)); }
        }

        public bool IsFire
        {
            get => _isFire;
            set { _isFire = value; OnPropertyChanged(nameof(IsFire)); }
        }

        public ICommand FireCommand { get; }
        public ObservableCollection<string> LogMessages { get; } = new();
        

        public ICommand ModeChangedCommand { get; }

        public event PropertyChangedEventHandler? PropertyChanged;

        public MainViewModel(IDialogService dialogService)
        {
            ReceiverA = new ReceiverModel { SignalStrength = 0 };
            ReceiverB = new ReceiverModel { SignalStrength = 0 };
            SystemStatus = "발사 준비";
            StatusColor = Brushes.Blue;

            _dialogService = dialogService;
            Serial = new SerialCommunication(this);

            TryAutoConnect();

            FireCommand = new RelayCommand(async (_) => await FireAsync());
            ModeChangedCommand = new RelayCommand(ChangeMode);
        }

        private void TryAutoConnect()
        {
            string defaultPort = "COM3";

            bool connected = Serial.ConnectToPort(defaultPort);
            if (connected)
            {
                AddLog($"[System] {defaultPort} 자동 연결 성공");
                return;
            }

            AddLog($"[System] {defaultPort} 연결 실패 → 포트 선택 요청");

            string[] ports = SerialPort.GetPortNames();
            if (ports.Length == 0)
            {
                _dialogService.ShowMessage("사용 가능한 포트가 없습니다.");
                return;
            }

            string? selectedPort = _dialogService.SelectSerialPort(ports);
            if (string.IsNullOrEmpty(selectedPort))
            {
                _dialogService.ShowMessage("포트 선택이 취소되었습니다.");
                return;
            }

            connected = Serial.ConnectToPort(selectedPort);
            if (connected)
            {
                AddLog($"[System] {selectedPort} 연결 성공");
            }
            else
            {
                _dialogService.ShowMessage($"{selectedPort} 포트 연결에 실패했습니다.");
            }
        }

        private async Task FireAsync()
        {
            if (IsFire) return; // 중복 방지
            IsFire = true;

            SystemStatus = "발사 진행 중...";
            StatusColor = Brushes.Orange;

            bool result = await Serial.SendFireCommandAsync(timeoutMilliseconds: 3000); // 발사 프로세스

            if (result)
            {
                SystemStatus = "발사 성공!";
                StatusColor = Brushes.Green;
            }
            else
            {
                SystemStatus = "발사 실패!";
                StatusColor = Brushes.Red;
            }

            IsFire = false;
        }

        public void UpdateReceiverColors()
        {
            if(Diff > 0)
            {
                ReceiverAColor = Brushes.LightGreen;
                ReceiverBColor = Brushes.LightGray;

            } else if(Diff < 0)
            {
                ReceiverAColor = Brushes.LightGray;
                ReceiverBColor = Brushes.LightGreen;
            } 
            else
            {
                ReceiverAColor = Brushes.LightYellow;
                ReceiverBColor = Brushes.LightYellow;
            }
        }
        private void ChangeMode(object parameter)
        {
            IsManualMode = (bool)parameter;
        }

        public void AddLog(string message)
        {
            Application.Current.Dispatcher.Invoke(() =>
            {
                LogMessages.Add($"{DateTime.Now:HH:mm:ss} - {message}");
            });
        }

        protected virtual void OnPropertyChanged(string propertyName)
        {
            PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(propertyName));
        }
    }
}