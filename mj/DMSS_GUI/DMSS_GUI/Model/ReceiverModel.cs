using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace DMSS_GUI.Model
{
    public class ReceiverModel : INotifyPropertyChanged
    {
        private double _signalStrength;

        public double SignalStrength
        {
            get => _signalStrength;
            set
            {
                _signalStrength = value;
                OnPropertyChanged(nameof(SignalStrength));
            }
        }

        public event PropertyChangedEventHandler? PropertyChanged;

        protected virtual void OnPropertyChanged(string propertyName)
        {
            PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(propertyName));
        }
    }
}
