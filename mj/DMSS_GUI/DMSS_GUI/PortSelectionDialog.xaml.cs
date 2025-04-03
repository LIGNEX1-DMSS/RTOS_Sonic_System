using DMSS_GUI.ViewModel;
using System;
using System.Collections.Generic;
using System.IO.Ports;
using System.Linq;
using System.Runtime.InteropServices.Marshalling;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Shapes;

namespace DMSS_GUI
{
    /// <summary>
    /// PortSelectionDialog.xaml에 대한 상호 작용 논리
    /// </summary>
    public partial class PortSelectionDialog : Window
    {
        public string? SelectedPort => (DataContext as PortSelectionDialogViewModel)?.SelectedPort;
        public PortSelectionDialog(IEnumerable<string> ports)
        {
            InitializeComponent();

            var viewModel = new PortSelectionDialogViewModel(ports);

            viewModel.CloseAction = (result) =>
            {
                DialogResult = result;
                Close();
            };

            DataContext = viewModel;
        }
    }
}
