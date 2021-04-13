namespace ErrorDialog
{
    partial class CodeMessage
    {
        /// <summary>
        /// Required designer variable.
        /// </summary>
        private System.ComponentModel.IContainer components = null;

        /// <summary>
        /// Clean up any resources being used.
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
        /// Required method for Designer support - do not modify
        /// the contents of this method with the code editor.
        /// </summary>
        private void InitializeComponent()
        {
            System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(CodeMessage));
            this.MessageText = new System.Windows.Forms.TextBox();
            this.Message = new System.Windows.Forms.Label();
            this.pictureBox1 = new System.Windows.Forms.PictureBox();
            this.DebugBreak = new System.Windows.Forms.Button();
            this.Continue = new System.Windows.Forms.Button();
            this.DataGrid = new System.Windows.Forms.PropertyGrid();
            this.Ignore = new System.Windows.Forms.Button();
            ((System.ComponentModel.ISupportInitialize)(this.pictureBox1)).BeginInit();
            this.SuspendLayout();
            // 
            // MessageText
            // 
            this.MessageText.Font = new System.Drawing.Font("Microsoft Sans Serif", 9.75F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.MessageText.Location = new System.Drawing.Point(97, 28);
            this.MessageText.Multiline = true;
            this.MessageText.Name = "MessageText";
            this.MessageText.ScrollBars = System.Windows.Forms.ScrollBars.Vertical;
            this.MessageText.Size = new System.Drawing.Size(567, 57);
            this.MessageText.TabIndex = 3;
            this.MessageText.TabStop = false;
            // 
            // Message
            // 
            this.Message.AutoSize = true;
            this.Message.Location = new System.Drawing.Point(94, 12);
            this.Message.Name = "Message";
            this.Message.Size = new System.Drawing.Size(53, 13);
            this.Message.TabIndex = 6;
            this.Message.Text = "Message:";
            // 
            // pictureBox1
            // 
            this.pictureBox1.ErrorImage = ((System.Drawing.Image)(resources.GetObject("pictureBox1.ErrorImage")));
            this.pictureBox1.Image = ((System.Drawing.Image)(resources.GetObject("pictureBox1.Image")));
            this.pictureBox1.InitialImage = ((System.Drawing.Image)(resources.GetObject("pictureBox1.InitialImage")));
            this.pictureBox1.Location = new System.Drawing.Point(12, 12);
            this.pictureBox1.Name = "pictureBox1";
            this.pictureBox1.Size = new System.Drawing.Size(66, 73);
            this.pictureBox1.TabIndex = 8;
            this.pictureBox1.TabStop = false;
            // 
            // DebugBreak
            // 
            this.DebugBreak.Image = ((System.Drawing.Image)(resources.GetObject("DebugBreak.Image")));
            this.DebugBreak.ImageAlign = System.Drawing.ContentAlignment.MiddleLeft;
            this.DebugBreak.Location = new System.Drawing.Point(511, 259);
            this.DebugBreak.Name = "DebugBreak";
            this.DebugBreak.Size = new System.Drawing.Size(153, 40);
            this.DebugBreak.TabIndex = 0;
            this.DebugBreak.Text = "Debug Break";
            this.DebugBreak.UseVisualStyleBackColor = true;
            this.DebugBreak.Click += new System.EventHandler(this.DebugBreak_Click);
            // 
            // Continue
            // 
            this.Continue.Image = ((System.Drawing.Image)(resources.GetObject("Continue.Image")));
            this.Continue.ImageAlign = System.Drawing.ContentAlignment.MiddleLeft;
            this.Continue.Location = new System.Drawing.Point(352, 259);
            this.Continue.Name = "Continue";
            this.Continue.Size = new System.Drawing.Size(153, 40);
            this.Continue.TabIndex = 1;
            this.Continue.Text = "Continue";
            this.Continue.UseVisualStyleBackColor = true;
            this.Continue.Click += new System.EventHandler(this.Continue_Click);
            // 
            // DataGrid
            // 
            this.DataGrid.CategoryForeColor = System.Drawing.SystemColors.InactiveCaptionText;
            this.DataGrid.CommandsDisabledLinkColor = System.Drawing.Color.Black;
            this.DataGrid.Font = new System.Drawing.Font("Microsoft Sans Serif", 9F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.DataGrid.Location = new System.Drawing.Point(12, 91);
            this.DataGrid.Name = "DataGrid";
            this.DataGrid.PropertySort = System.Windows.Forms.PropertySort.NoSort;
            this.DataGrid.Size = new System.Drawing.Size(652, 162);
            this.DataGrid.TabIndex = 2;
            this.DataGrid.TabStop = false;
            this.DataGrid.ToolbarVisible = false;
            // 
            // Ignore
            // 
            this.Ignore.Image = ((System.Drawing.Image)(resources.GetObject("Ignore.Image")));
            this.Ignore.ImageAlign = System.Drawing.ContentAlignment.MiddleLeft;
            this.Ignore.Location = new System.Drawing.Point(12, 259);
            this.Ignore.Name = "Ignore";
            this.Ignore.Size = new System.Drawing.Size(153, 40);
            this.Ignore.TabIndex = 9;
            this.Ignore.Text = "Ignore";
            this.Ignore.UseVisualStyleBackColor = true;
            this.Ignore.Click += new System.EventHandler(this.Ignore_Click);
            // 
            // CodeMessage
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.BackgroundImageLayout = System.Windows.Forms.ImageLayout.None;
            this.ClientSize = new System.Drawing.Size(678, 306);
            this.Controls.Add(this.Ignore);
            this.Controls.Add(this.DebugBreak);
            this.Controls.Add(this.Continue);
            this.Controls.Add(this.DataGrid);
            this.Controls.Add(this.pictureBox1);
            this.Controls.Add(this.Message);
            this.Controls.Add(this.MessageText);
            this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.FixedSingle;
            this.Icon = ((System.Drawing.Icon)(resources.GetObject("$this.Icon")));
            this.MaximizeBox = false;
            this.Name = "CodeMessage";
            this.StartPosition = System.Windows.Forms.FormStartPosition.CenterScreen;
            this.Text = "Zero Engine Error";
            this.TopMost = true;
            this.KeyPress += new System.Windows.Forms.KeyPressEventHandler(this.CodeMessage_KeyPress);
            ((System.ComponentModel.ISupportInitialize)(this.pictureBox1)).EndInit();
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.TextBox MessageText;
        private System.Windows.Forms.Label Message;
        private System.Windows.Forms.PictureBox pictureBox1;
        private System.Windows.Forms.Button DebugBreak;
        private System.Windows.Forms.Button Continue;
        private System.Windows.Forms.PropertyGrid DataGrid;
        private System.Windows.Forms.Button Ignore;

    }
}