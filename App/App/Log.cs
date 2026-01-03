using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace App
{
    class Log
    {
        public static void AddLog(string text, Color color)
        {
            string timeStamp = DateTime.Now.ToString("[HH:mm:ss.f] ");

            Form1.LogsBox.SelectionColor = color;
            Form1.LogsBox.AppendText(timeStamp + text + "\n");
        }
    }
}
