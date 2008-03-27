namespace GIDFit
{
    partial class Pubgraph
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
            this.components = new System.ComponentModel.Container();
            this.GIDGraph = new ZedGraph.ZedGraphControl();
            this.SuspendLayout();
            // 
            // GIDGraph
            // 
            this.GIDGraph.Anchor = System.Windows.Forms.AnchorStyles.None;
            this.GIDGraph.EditButtons = System.Windows.Forms.MouseButtons.Left;
            this.GIDGraph.Location = new System.Drawing.Point(-5, -2);
            this.GIDGraph.Name = "GIDGraph";
            this.GIDGraph.PanModifierKeys = ((System.Windows.Forms.Keys)((System.Windows.Forms.Keys.Shift | System.Windows.Forms.Keys.None)));
            this.GIDGraph.ScrollGrace = 0;
            this.GIDGraph.ScrollMaxX = 0;
            this.GIDGraph.ScrollMaxY = 0;
            this.GIDGraph.ScrollMaxY2 = 0;
            this.GIDGraph.ScrollMinX = 0;
            this.GIDGraph.ScrollMinY = 0;
            this.GIDGraph.ScrollMinY2 = 0;
            this.GIDGraph.Size = new System.Drawing.Size(559, 340);
            this.GIDGraph.TabIndex = 11;
            // 
            // Pubgraph
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(96F, 96F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Dpi;
            this.ClientSize = new System.Drawing.Size(550, 337);
            this.Controls.Add(this.GIDGraph);
            this.MaximizeBox = false;
            this.Name = "Pubgraph";
            this.Text = "Pubgraph";
            this.ResumeLayout(false);

        }

        #endregion

        private ZedGraph.ZedGraphControl GIDGraph;

    }
}