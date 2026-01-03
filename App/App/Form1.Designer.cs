namespace App
{
    partial class Form1
    {
        /// <summary>
        ///  Required designer variable.
        /// </summary>
        private System.ComponentModel.IContainer components = null;

        /// <summary>
        ///  Clean up any resources being used.
        /// </summary>
        /// <param name="disposing">true if managed resources should be disposed; otherwise, false.</param>
        protected override void Dispose(bool disposing)
        {
            if (disposing && (components != null))
            {
                components.Dispose();
            }
            base.Dispose(disposing);
        }

        #region Windows Form Designer generated code

        /// <summary>
        ///  Required method for Designer support - do not modify
        ///  the contents of this method with the code editor.
        /// </summary>
        private void InitializeComponent()
        {
            components = new System.ComponentModel.Container();
            panel1 = new Panel();
            startProtectionButton = new Button();
            label1 = new Label();
            panel2 = new Panel();
            groupBox1 = new GroupBox();
            detectionsCount = new Label();
            label4 = new Label();
            driverStatus = new Label();
            label2 = new Label();
            panel3 = new Panel();
            tabControl1 = new TabControl();
            detectionsPage = new TabPage();
            detectionsBox = new ListView();
            timeColumn = new ColumnHeader();
            fullCommandColumn = new ColumnHeader();
            contextMenuStrip1 = new ContextMenuStrip(components);
            copyToolStripMenuItem = new ToolStripMenuItem();
            logsPage = new TabPage();
            logsBox = new RichTextBox();
            statusStrip1 = new StatusStrip();
            toolStripStatusLabel1 = new ToolStripStatusLabel();
            imageList1 = new ImageList(components);
            panel1.SuspendLayout();
            panel2.SuspendLayout();
            groupBox1.SuspendLayout();
            panel3.SuspendLayout();
            tabControl1.SuspendLayout();
            detectionsPage.SuspendLayout();
            contextMenuStrip1.SuspendLayout();
            logsPage.SuspendLayout();
            statusStrip1.SuspendLayout();
            SuspendLayout();
            // 
            // panel1
            // 
            panel1.BackColor = Color.FromArgb(37, 165, 250);
            panel1.Controls.Add(startProtectionButton);
            panel1.Controls.Add(label1);
            panel1.Dock = DockStyle.Top;
            panel1.Location = new Point(0, 0);
            panel1.Name = "panel1";
            panel1.Padding = new Padding(10);
            panel1.Size = new Size(1081, 61);
            panel1.TabIndex = 0;
            // 
            // startProtectionButton
            // 
            startProtectionButton.Dock = DockStyle.Right;
            startProtectionButton.Location = new Point(964, 10);
            startProtectionButton.Name = "startProtectionButton";
            startProtectionButton.Size = new Size(107, 41);
            startProtectionButton.TabIndex = 1;
            startProtectionButton.Text = "Start protection";
            startProtectionButton.UseVisualStyleBackColor = true;
            startProtectionButton.Click += button1_Click;
            // 
            // label1
            // 
            label1.Dock = DockStyle.Left;
            label1.Font = new Font("Segoe UI", 15.75F, FontStyle.Bold, GraphicsUnit.Point, 0);
            label1.ForeColor = Color.White;
            label1.Location = new Point(10, 10);
            label1.Name = "label1";
            label1.Size = new Size(122, 41);
            label1.TabIndex = 0;
            label1.Text = "LoLScan";
            label1.TextAlign = ContentAlignment.MiddleCenter;
            // 
            // panel2
            // 
            panel2.BackColor = SystemColors.ControlLight;
            panel2.Controls.Add(groupBox1);
            panel2.Dock = DockStyle.Left;
            panel2.Location = new Point(0, 61);
            panel2.Margin = new Padding(0);
            panel2.Name = "panel2";
            panel2.Padding = new Padding(10);
            panel2.Size = new Size(270, 454);
            panel2.TabIndex = 1;
            // 
            // groupBox1
            // 
            groupBox1.Controls.Add(detectionsCount);
            groupBox1.Controls.Add(label4);
            groupBox1.Controls.Add(driverStatus);
            groupBox1.Controls.Add(label2);
            groupBox1.Dock = DockStyle.Top;
            groupBox1.Location = new Point(10, 10);
            groupBox1.Name = "groupBox1";
            groupBox1.Size = new Size(250, 115);
            groupBox1.TabIndex = 0;
            groupBox1.TabStop = false;
            groupBox1.Text = "Statistics";
            // 
            // detectionsCount
            // 
            detectionsCount.AutoSize = true;
            detectionsCount.Font = new Font("Segoe UI Semibold", 12F, FontStyle.Bold, GraphicsUnit.Point, 0);
            detectionsCount.ForeColor = Color.Green;
            detectionsCount.Location = new Point(132, 64);
            detectionsCount.Name = "detectionsCount";
            detectionsCount.Size = new Size(19, 21);
            detectionsCount.TabIndex = 3;
            detectionsCount.Text = "0";
            detectionsCount.TextAlign = ContentAlignment.MiddleLeft;
            // 
            // label4
            // 
            label4.AutoSize = true;
            label4.Font = new Font("Segoe UI", 12F, FontStyle.Bold, GraphicsUnit.Point, 0);
            label4.Location = new Point(6, 64);
            label4.Name = "label4";
            label4.Size = new Size(92, 21);
            label4.TabIndex = 2;
            label4.Text = "Detections";
            // 
            // driverStatus
            // 
            driverStatus.AutoSize = true;
            driverStatus.Font = new Font("Segoe UI Semibold", 12F, FontStyle.Bold, GraphicsUnit.Point, 0);
            driverStatus.ForeColor = Color.DarkGreen;
            driverStatus.Location = new Point(90, 33);
            driverStatus.Name = "driverStatus";
            driverStatus.Size = new Size(32, 21);
            driverStatus.TabIndex = 1;
            driverStatus.Text = "OK";
            // 
            // label2
            // 
            label2.AutoSize = true;
            label2.Font = new Font("Segoe UI", 12F, FontStyle.Bold, GraphicsUnit.Point, 0);
            label2.Location = new Point(6, 33);
            label2.Name = "label2";
            label2.Size = new Size(57, 21);
            label2.TabIndex = 0;
            label2.Text = "Driver";
            // 
            // panel3
            // 
            panel3.Anchor = AnchorStyles.Top | AnchorStyles.Bottom | AnchorStyles.Left | AnchorStyles.Right;
            panel3.BackColor = SystemColors.Control;
            panel3.Controls.Add(tabControl1);
            panel3.Location = new Point(270, 61);
            panel3.Margin = new Padding(0);
            panel3.Name = "panel3";
            panel3.Padding = new Padding(10);
            panel3.Size = new Size(811, 454);
            panel3.TabIndex = 2;
            // 
            // tabControl1
            // 
            tabControl1.Anchor = AnchorStyles.Top | AnchorStyles.Bottom | AnchorStyles.Left | AnchorStyles.Right;
            tabControl1.Controls.Add(detectionsPage);
            tabControl1.Controls.Add(logsPage);
            tabControl1.Location = new Point(10, 10);
            tabControl1.Name = "tabControl1";
            tabControl1.SelectedIndex = 0;
            tabControl1.Size = new Size(791, 434);
            tabControl1.TabIndex = 0;
            // 
            // detectionsPage
            // 
            detectionsPage.Controls.Add(detectionsBox);
            detectionsPage.Location = new Point(4, 24);
            detectionsPage.Name = "detectionsPage";
            detectionsPage.Padding = new Padding(3);
            detectionsPage.Size = new Size(783, 406);
            detectionsPage.TabIndex = 0;
            detectionsPage.Text = "Detections";
            detectionsPage.UseVisualStyleBackColor = true;
            // 
            // detectionsBox
            // 
            detectionsBox.Columns.AddRange(new ColumnHeader[] { timeColumn, fullCommandColumn });
            detectionsBox.ContextMenuStrip = contextMenuStrip1;
            detectionsBox.Dock = DockStyle.Fill;
            detectionsBox.FullRowSelect = true;
            detectionsBox.GridLines = true;
            detectionsBox.ImeMode = ImeMode.NoControl;
            detectionsBox.Location = new Point(3, 3);
            detectionsBox.MultiSelect = false;
            detectionsBox.Name = "detectionsBox";
            detectionsBox.Size = new Size(777, 400);
            detectionsBox.TabIndex = 0;
            detectionsBox.UseCompatibleStateImageBehavior = false;
            detectionsBox.View = View.Details;
            // 
            // timeColumn
            // 
            timeColumn.Text = "Time";
            timeColumn.Width = 100;
            // 
            // fullCommandColumn
            // 
            fullCommandColumn.Text = "Full command";
            fullCommandColumn.Width = 670;
            // 
            // contextMenuStrip1
            // 
            contextMenuStrip1.Items.AddRange(new ToolStripItem[] { copyToolStripMenuItem });
            contextMenuStrip1.Name = "contextMenuStrip1";
            contextMenuStrip1.RenderMode = ToolStripRenderMode.System;
            contextMenuStrip1.ShowImageMargin = false;
            contextMenuStrip1.Size = new Size(120, 26);
            // 
            // copyToolStripMenuItem
            // 
            copyToolStripMenuItem.DisplayStyle = ToolStripItemDisplayStyle.Text;
            copyToolStripMenuItem.Name = "copyToolStripMenuItem";
            copyToolStripMenuItem.ShortcutKeys = Keys.Control | Keys.C;
            copyToolStripMenuItem.Size = new Size(119, 22);
            copyToolStripMenuItem.Text = "Copy";
            copyToolStripMenuItem.Click += copyToolStripMenuItem_Click;
            // 
            // logsPage
            // 
            logsPage.Controls.Add(logsBox);
            logsPage.Location = new Point(4, 24);
            logsPage.Name = "logsPage";
            logsPage.Padding = new Padding(3);
            logsPage.Size = new Size(783, 406);
            logsPage.TabIndex = 1;
            logsPage.Text = "Logs";
            logsPage.UseVisualStyleBackColor = true;
            // 
            // logsBox
            // 
            logsBox.BackColor = SystemColors.Window;
            logsBox.Dock = DockStyle.Fill;
            logsBox.Font = new Font("Consolas", 9.75F, FontStyle.Regular, GraphicsUnit.Point, 0);
            logsBox.Location = new Point(3, 3);
            logsBox.Name = "logsBox";
            logsBox.ReadOnly = true;
            logsBox.Size = new Size(777, 400);
            logsBox.TabIndex = 0;
            logsBox.Text = "";
            logsBox.TextChanged += logsBox_TextChanged;
            // 
            // statusStrip1
            // 
            statusStrip1.Items.AddRange(new ToolStripItem[] { toolStripStatusLabel1 });
            statusStrip1.Location = new Point(0, 515);
            statusStrip1.Name = "statusStrip1";
            statusStrip1.Size = new Size(1081, 22);
            statusStrip1.TabIndex = 0;
            statusStrip1.Text = "statusStrip1";
            // 
            // toolStripStatusLabel1
            // 
            toolStripStatusLabel1.Name = "toolStripStatusLabel1";
            toolStripStatusLabel1.Size = new Size(39, 17);
            toolStripStatusLabel1.Text = "Ready";
            // 
            // imageList1
            // 
            imageList1.ColorDepth = ColorDepth.Depth32Bit;
            imageList1.ImageSize = new Size(16, 16);
            imageList1.TransparentColor = Color.Transparent;
            // 
            // Form1
            // 
            AutoScaleDimensions = new SizeF(7F, 15F);
            AutoScaleMode = AutoScaleMode.Font;
            ClientSize = new Size(1081, 537);
            Controls.Add(panel3);
            Controls.Add(panel2);
            Controls.Add(panel1);
            Controls.Add(statusStrip1);
            MinimumSize = new Size(1097, 576);
            Name = "Form1";
            Text = "Form1";
            Load += Form1_Load;
            panel1.ResumeLayout(false);
            panel2.ResumeLayout(false);
            groupBox1.ResumeLayout(false);
            groupBox1.PerformLayout();
            panel3.ResumeLayout(false);
            tabControl1.ResumeLayout(false);
            detectionsPage.ResumeLayout(false);
            contextMenuStrip1.ResumeLayout(false);
            logsPage.ResumeLayout(false);
            statusStrip1.ResumeLayout(false);
            statusStrip1.PerformLayout();
            ResumeLayout(false);
            PerformLayout();
        }

        #endregion

        private Panel panel1;
        private Label label1;
        private Button startProtectionButton;
        private Panel panel2;
        private Panel panel3;
        private StatusStrip statusStrip1;
        private ToolStripStatusLabel toolStripStatusLabel1;
        private GroupBox groupBox1;
        private Label label2;
        private Label label4;
        private Label driverStatus;
        private Label detectionsCount;
        private TabControl tabControl1;
        private TabPage detectionsPage;
        private TabPage logsPage;
        private ImageList imageList1;
        private ListView detectionsBox;
        private ColumnHeader timeColumn;
        private ColumnHeader fullCommandColumn;
        private RichTextBox logsBox;
        private ContextMenuStrip contextMenuStrip1;
        private ToolStripMenuItem copyToolStripMenuItem;
    }
}
