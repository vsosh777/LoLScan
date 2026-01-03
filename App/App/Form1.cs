using System.Windows.Forms;

namespace App
{
    public partial class Form1 : Form
    {
        public static RichTextBox LogsBox;
        public static Label DriverStatus;
        public static ListView DetectionsBox;
        public static Label DetectionsCount;

        public static bool protectionActive = false;
        private Driver driver;

        public Form1()
        {
            InitializeComponent();

            LogsBox = logsBox;
            DriverStatus = driverStatus;
            DetectionsBox = detectionsBox;
            DetectionsCount = detectionsCount;
        }

        private async void Form1_Load(object sender, EventArgs e)
        {
            Log.AddLog("Application loaded", Color.Blue);
            driver = new Driver();

            int total = Payloads.payloads.Count;
            Log.AddLog($"Loading {total} commands...", Color.Blue);
            
            await Task.Run(() =>
            {
                int count = 0;
                foreach (string p in Payloads.payloads)
                {
                    driver.AddCommand(p);
                    count++;
                    
                    //this.Invoke((Action)(() =>
                    //{
                    //    //Log.AddLog($"Loaded {count}/{total} commands", Color.Blue);
                    //}));
                }
            });

            Log.AddLog("All commands loaded successfully", Color.Green);
            driver.StartLogSubscriber(this);
        }

        private async void button1_Click(object sender, EventArgs e)
        {
            if (!protectionActive)
            {
                Log.AddLog("Protection started", Color.Green);
                startProtectionButton.Text = "Stop protection";
                protectionActive = true;
                driver.ToggleProt(protectionActive);
            }
            else
            {
                Log.AddLog("Protection stopped", Color.Red);
                startProtectionButton.Text = "Start protection";
                protectionActive = false;
                driver.ToggleProt(protectionActive);
            }
        }

        private void logsBox_TextChanged(object sender, EventArgs e)
        {
            logsBox.ScrollToCaret();
        }

        protected override void OnFormClosing(FormClosingEventArgs e)
        {
            base.OnFormClosing(e);

            if (driver != null)
            {
                try
                {
                    Log.AddLog("Closing driver connection...", Color.Blue);
                    Application.DoEvents();
                }
                catch { }

                driver.Dispose();
                driver = null;
            }
        }

        private void copyToolStripMenuItem_Click(object sender, EventArgs e)
        {
            Clipboard.SetText(detectionsBox.SelectedItems[0].SubItems[1].Text);
        }
    }
}
