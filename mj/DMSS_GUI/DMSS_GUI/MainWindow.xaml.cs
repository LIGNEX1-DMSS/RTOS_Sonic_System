using System.Text;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Threading;
using System.Windows.Media.Imaging;
using System.Windows.Navigation;
using System.Windows.Shapes;
using DMSS_GUI.ViewModel;

namespace DMSS_GUI
{
    /// <summary>
    /// Interaction logic for MainWindow.xaml
    /// </summary>
    public partial class MainWindow : Window
    {
        private MainViewModel _viewModel;
        private SerialCommunication _receiver;

        public MainWindow()
        {
            InitializeComponent();
            _viewModel = new MainViewModel(new DialogService());
            DataContext = _viewModel; // ViewModel을 DataContext로 설정

            _receiver = new SerialCommunication(_viewModel); //시리얼 연결
        }

        protected override void OnClosed(EventArgs e)
        {
            _receiver.Close(); //시리얼 포트 닫기
            base.OnClosed(e);
        }
    }
}