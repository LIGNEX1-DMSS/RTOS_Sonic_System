using DMSS_GUI.Interface;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;

namespace DMSS_GUI
{
    public class DialogService : IDialogService
    {
        public string? SelectSerialPort(IEnumerable<string> ports)
        {
            var dialog = new PortSelectionDialog(ports);
            bool? result = dialog.ShowDialog();
            return result == true ? dialog.SelectedPort : null;
        }

        public void ShowMessage(string message, string title = "알림")
        {
            MessageBox.Show(message, title);
        }
    }
}
