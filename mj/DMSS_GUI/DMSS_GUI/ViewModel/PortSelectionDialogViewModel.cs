using DMSS_GUI.command;
using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.ComponentModel;
using System.IO.Ports;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Input;

namespace DMSS_GUI.ViewModel
{
    public class PortSelectionDialogViewModel : INotifyPropertyChanged
    {
        public event PropertyChangedEventHandler? PropertyChanged;

        public ObservableCollection<string> availablePorts { get; } = new();
        private string? _selectedPort;
        public string? SelectedPort
        {
            get => _selectedPort;
            set { _selectedPort = value; OnPropertyChanged(nameof(SelectedPort)); }
        }

        public ICommand ConfirmCommand { get; }

        public Action<bool?>? CloseAction { get; set; } // 뷰에서 호출될 수 있도록 등록

        public PortSelectionDialogViewModel(IEnumerable<string> ports)
        {
            foreach(var port in ports)
            {
                availablePorts.Add(port);
            }
            SelectedPort = availablePorts.FirstOrDefault();
            ConfirmCommand = new RelayCommand(_ => CloseAction?.Invoke(true));
        }


        protected void OnPropertyChanged(string prop)
        {
            PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(prop));
        }
    }
}
