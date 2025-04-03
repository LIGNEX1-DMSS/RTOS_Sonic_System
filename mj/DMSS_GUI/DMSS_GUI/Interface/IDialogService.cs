using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace DMSS_GUI.Interface
{
    public interface IDialogService
    {
        string? SelectSerialPort(IEnumerable<string> ports);
        void ShowMessage(string message, string title = "알림");
    }
}
