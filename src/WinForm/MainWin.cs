using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Diagnostics;
using System.Drawing;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace WinForm
{
    public partial class MainWin : Form
    {
        [DllImport("HookDLL.dll")]
        private extern static short HK_Initialize(int pid);

        [DllImport("HookDLL.dll", CharSet = CharSet.Unicode)]
        private extern static short HK_AppendHookNode(string FileName, string FuncName);

        [DllImport("HookDLL.dll", CharSet = CharSet.Unicode)]
        private extern static short HK_RemoveHookNode(string FileName, string FuncName);

        [DllImport("HookDLL.dll", CharSet = CharSet.Unicode)]
        private extern static short HK_EnableOnceHook(string FileName, string FuncName);




        public MainWin()
        {
            InitializeComponent();
        }

        private void button1_Click(object sender, EventArgs e)
        {
            int pid = Process.GetProcessesByName("notepad")[0].Id;





            HK_Initialize(pid);
            HK_AppendHookNode("Kernel32.dll", "HeapCreate");
            HK_EnableOnceHook("Kernel32.dll", "HeapCreate");
        }
    }
}
