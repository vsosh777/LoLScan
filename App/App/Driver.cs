using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;
using System.Threading; // NEW

namespace App
{
    class Driver : IDisposable // NEW: IDisposable
    {
        static uint CTL_CODE(uint DeviceType, uint Function, uint Method, uint Access)
        {
            return ((DeviceType << 16) | (Access << 14) | (Function << 2) | Method);
        }

        const uint FILE_ANY_ACCESS = 0;
        const uint METHOD_BUFFERED = 0;
        const uint SIOCTL_TYPE = 40000;
        static uint IOCTL_HI = CTL_CODE(SIOCTL_TYPE, 0x900, METHOD_BUFFERED, FILE_ANY_ACCESS);
        static uint IOCTL_SUBSCRIBE_LOG = CTL_CODE(SIOCTL_TYPE, 0x901, METHOD_BUFFERED, FILE_ANY_ACCESS);
        static uint IOCTL_ADD_RULE = CTL_CODE(SIOCTL_TYPE, 0x902, METHOD_BUFFERED, FILE_ANY_ACCESS);
        static uint IOCTL_TOGGLE_PROT = CTL_CODE(SIOCTL_TYPE, 0x903, METHOD_BUFFERED, FILE_ANY_ACCESS);

        [DllImport("kernel32.dll", CharSet = CharSet.Auto, SetLastError = true)]
        private static extern IntPtr CreateFile(string filename, FileAccess access,
              FileShare sharing, IntPtr SecurityAttributes, FileMode mode,
              FileOptions options, IntPtr template
        );

        [DllImport("kernel32.dll", SetLastError = true)]
        private static extern bool DeviceIoControl(IntPtr device, uint ctlcode,
            byte[] inbuffer, int inbuffersize,
            byte[] outbuffer, int outbufferSize,
            out uint bytesreturned, IntPtr overlapped
        );

        [DllImport("kernel32.dll")]
        private static extern void CloseHandle(IntPtr hdl);

        [DllImport("kernel32.dll", SetLastError = true)]
        private static extern bool CancelIoEx(IntPtr hFile, IntPtr lpOverlapped);

        private IntPtr hdl;
        private CancellationTokenSource cancellationTokenSource;
        private Task subscriberTask;
        private volatile bool isDisposing = false;
        private volatile bool pauseSubscriber = false;
        private object ioctlLock = new object();

        public Driver()
        {
            hdl = CreateFile("\\\\.\\LoLScanDRV", FileAccess.ReadWrite,
            FileShare.None, IntPtr.Zero, FileMode.Open,
            FileOptions.None, IntPtr.Zero);
            if (hdl == (IntPtr)(-1))
            {
                Form1.DriverStatus.Text = "Error";
                Form1.DriverStatus.ForeColor = Color.Red;
                Log.AddLog("Error connecting to driver", Color.Red);
                return;
            }
            try
            {
                byte[] input = System.Text.Encoding.UTF8.GetBytes("Hi");
                byte[] output = new byte[64];
                bool ok = DeviceIoControl(hdl, IOCTL_HI, input, input.Length, output,
                    output.Length, out uint bytesReturned, IntPtr.Zero);

                Log.AddLog("Connected to driver succesfully", Color.Green);
                Log.AddLog("Output from driver: " + System.Text.Encoding.UTF8.GetString(output).TrimEnd('\0'), Color.Green);
                if (!ok) throw new Win32Exception();
            }
            catch
            {
            }
        }

        public void AddCommand(string cmd)
        {
            byte[] input2 = System.Text.Encoding.UTF8.GetBytes(cmd);
            byte[] output2 = new byte[64];
            bool ok2 = DeviceIoControl(hdl, IOCTL_ADD_RULE, input2, input2.Length, output2,
                output2.Length, out uint bytesReturned2, IntPtr.Zero);

            if (!ok2) throw new Win32Exception();
        }

        public void ToggleProt(bool active)
        {
            // Stop subscriber and close handle
            var oldHandle = hdl;
            cancellationTokenSource?.Cancel();
            
            Task.Run(() =>
            {
                Thread.Sleep(100);
                if (oldHandle != IntPtr.Zero && oldHandle != (IntPtr)(-1))
                {
                    CloseHandle(oldHandle);
                }
            });
            
            Thread.Sleep(200);
            
            // Reopen driver
            hdl = CreateFile("\\\\.\\LoLScanDRV", FileAccess.ReadWrite,
                FileShare.None, IntPtr.Zero, FileMode.Open,
                FileOptions.None, IntPtr.Zero);
                
            if (hdl == (IntPtr)(-1))
            {
                Log.AddLog("Error reconnecting to driver", Color.Red);
                return;
            }
            
            // Send toggle command
            byte[] input2 = System.Text.Encoding.UTF8.GetBytes("START");
            if (!active)
            {
                input2 = System.Text.Encoding.UTF8.GetBytes("STOP");
            }

            byte[] output2 = new byte[64];
            bool ok2 = DeviceIoControl(hdl, IOCTL_TOGGLE_PROT, input2, input2.Length, output2,
                output2.Length, out uint bytesReturned2, IntPtr.Zero);

            if (!ok2) throw new Win32Exception();
            
            // Restart subscriber
            cancellationTokenSource = new CancellationTokenSource();
            subscriberTask = Task.Run(() => SubscribeLoop(Form1.LogsBox?.FindForm(), cancellationTokenSource.Token));
        }

        public void StartLogSubscriber(Form asd)
        {
            cancellationTokenSource = new CancellationTokenSource();
            subscriberTask = Task.Run(() => SubscribeLoop(asd, cancellationTokenSource.Token));
        }

        private void SubscribeLoop(Form asd, CancellationToken cancellationToken)
        {
            if (hdl == (IntPtr)(-1))
            {
                return;
            }
            try
            {
                while (!cancellationToken.IsCancellationRequested)
                {
                    byte[] buffer = new byte[1024];
                    bool ok = DeviceIoControl(hdl, IOCTL_SUBSCRIBE_LOG,
                        null, 0, buffer, buffer.Length,
                        out uint bytesReturned, IntPtr.Zero);

                    if (cancellationToken.IsCancellationRequested)
                    {
                        break;
                    }

                    if (ok && bytesReturned > 0)
                    {
                        string logMsg = Encoding.Unicode.GetString(buffer, 0, (int)bytesReturned);

                        if (!isDisposing && asd != null && !asd.IsDisposed)
                        {
                            try
                            {
                                asd.BeginInvoke((Action)(() =>
                                {
                                    if (!asd.IsDisposed)
                                    {
                                        Log.AddLog(logMsg, Color.Red);

                                        if (logMsg.StartsWith("BLOCKED "))
                                        {
                                            string timeStamp = DateTime.Now.ToString("HH:mm:ss");

                                            ListViewItem item = new ListViewItem();
                                            item.Text = timeStamp;
                                            item.SubItems.Add(logMsg.Substring(8));

                                            Form1.DetectionsBox.Items.Add(item);

                                            Form1.DetectionsCount.Text = (int.Parse(Form1.DetectionsCount.Text) + 1).ToString();
                                            Form1.DetectionsCount.ForeColor = Color.Red;
                                        }
                                    }
                                }));
                            }
                            catch { }
                        }
                    }
                    else
                    {
                        int error = Marshal.GetLastWin32Error();
                        if (error == 995)
                        {
                            System.Threading.Thread.Sleep(50);
                            continue;
                        }
                        System.Threading.Thread.Sleep(100);
                    }
                }
            }
            finally
            {
            }
        }

        public void Dispose()
        {
            isDisposing = true;
            cancellationTokenSource?.Cancel();

            Task.Run(() =>
            {
                if (hdl != IntPtr.Zero && hdl != (IntPtr)(-1))
                {
                    CloseHandle(hdl);
                    hdl = IntPtr.Zero;
                }
            });

            cancellationTokenSource?.Dispose();
        }
    }
}