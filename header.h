
#ifndef HEADER_H
#define HEADER_H
#include <wx/wxprec.h>
#include <wx/dcbuffer.h>

#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>


struct coordinate_input {
    double latitude;
    double longitude;
    double time;

};

struct pixel_data {
    double value;
    double colour;

};

struct GPSCoordinate {
    double latitude;
    double longitude;
};

struct PixelCoordinate {
    int x;
    int y;
};

struct MapBounds {
    GPSCoordinate topLeft;
    GPSCoordinate topRight;
    GPSCoordinate bottomLeft;
    GPSCoordinate bottomRight;
};



double interpolate(std::vector<coordinate_input> input_data, double longitude, double latitude) {
    double dist;
    double weighted_sum = 0.0;
    double weight_total = 0.0;
    const double epsilon = 1e-6;
    for (const auto& point : input_data) {
        dist = std::sqrt((point.latitude - latitude) * (point.latitude - latitude) + (point.longitude - longitude) * (point.longitude - longitude));

        if (dist < epsilon) { // Handle case when the point is extremely close
            return point.time;
        }

        double weight = 1.0 / dist;
        weighted_sum += point.time * weight;
        weight_total += weight;
    }
    return (weight_total > 0) ? weighted_sum / weight_total : 0.0;
};


std::vector<coordinate_input> read_csv(std::string file_name) {
    std::vector<coordinate_input> input_data;
    int i = 0;
    // Open the CSV file
    std::ifstream file(file_name);
    if (!file.is_open()) {
        std::cerr << "Error: Could not open file." << std::endl;
        return input_data;
    }

    std::string line;
    // Skip the header line (if present)
    std::getline(file, line);

    // Read each line from the file
    while (std::getline(file, line)) {
        input_data.push_back(coordinate_input{});
        std::stringstream ss(line);
        std::string value;

        double doubleVal;

        // Column 1
        std::getline(ss, value, ',');
        doubleVal = std::stod(value);
        input_data[i].longitude = doubleVal;

        // Column 2
        std::getline(ss, value, ',');
        doubleVal = std::stod(value);
        input_data[i].latitude = doubleVal;

        // Column 3
        std::getline(ss, value, ',');
        doubleVal = std::stod(value);
        input_data[i].time = doubleVal;

        i++;
    }

    file.close();

    return input_data;
};


class MyApp : public wxApp
{
public:
    virtual bool OnInit();
};



// Function to convert pixel coordinates to GPS coordinates
GPSCoordinate pixelToGPS(
    const PixelCoordinate& pixel,
    const int imageWidth, const int imageHeight,
    const GPSCoordinate& topLeft, const GPSCoordinate& topRight,
    const GPSCoordinate& bottomLeft, const GPSCoordinate& bottomRight
) {
    // Normalize pixel coordinates to a [0, 1] range
    double x_ratio = static_cast<double>(pixel.x) / imageWidth;
    double y_ratio = static_cast<double>(pixel.y) / imageHeight;

    // Interpolate latitude and longitude
    double lat = topLeft.latitude
        + x_ratio * (topRight.latitude - topLeft.latitude)
        + y_ratio * (bottomLeft.latitude - topLeft.latitude);
    double lon = topLeft.longitude
        + x_ratio * (topRight.longitude - topLeft.longitude)
        + y_ratio * (bottomLeft.longitude - topLeft.longitude);

    return { lat, lon };
}


class MyFrame : public wxFrame
{
public:
    MyFrame(const std::vector<std::vector<pixel_data>>& data);

    MapBounds getMapBounds() const { return mapBounds; }

private:
    MapBounds mapBounds = {
    { -37.673467, 144.900807 }, // topLeft
    { -37.675475, 145.244541 }, // topRight
    { -37.933589, 144.913295 }, // bottomLeft
    { -37.931713, 145.237128 }  // bottomRight
    };
    void OnPaint(wxPaintEvent& event);
    void OnHello(wxCommandEvent& event);
    void OnExit(wxCommandEvent& event);
    void OnAbout(wxCommandEvent& event);
    void OnMouseWheel(wxMouseEvent& event);   // For zooming
    void OnLeftMouseDown(wxMouseEvent& event);    // For initiating panning
    void OnRightMouseDown(wxMouseEvent& event);    // For initiating panning
    void OnMouseMove(wxMouseEvent& event);    // For panning the map
    void OnMouseUp(wxMouseEvent& event);      // For ending panning
    void OnMouseClick(wxMouseEvent& event);   // For outputting coordinates on click

    wxBitmap m_backgroundImage;  // Bitmap to store the background image

    double m_zoomLevel = 1.0;                // Zoom factor
    wxPoint m_panOffset = wxPoint(0, 0);     // Offset for panning
    wxPoint m_lastMousePos;                  // Stores the last mouse position for panning

    bool m_isPanning = false;                // Flag for panning mode

    std::vector<std::vector<pixel_data>> image_data;
};

enum
{
    ID_Hello = 1
};

wxIMPLEMENT_APP(MyApp);

bool MyApp::OnInit()
{

    const std::string filename_input = "C:/Users/michael.h_chemwatch/source/repos/commute/test_coords.csv";
    PixelCoordinate data_point;
    GPSCoordinate gps;
    MapBounds mapBounds = {
    { -37.673467, 144.900807 }, // topLeft
    { -37.675475, 145.244541 }, // topRight
    { -37.933589, 144.913295 }, // bottomLeft
    { -37.931713, 145.237128 }  // bottomRight
    };

    int image_width = 1448;
    int image_height = 1340;
    

    std::vector<std::vector<pixel_data>> image_data(image_height, std::vector<pixel_data>(image_width));
    //image_data.resize(image_height, std::vector<pixel_data>(image_width));
    //update
    //std::vector<DataPoint> data = readCSV(filename);
    std::vector<coordinate_input> input_data;
    input_data = read_csv(filename_input);
    //for (int i = 0; i < input_data.size(); i++)
    //{
    //    std::cout << input_data[i].longitude << std::endl;
    //}

    for (int i = 0; i < image_data.size(); ++i) { // Loop over rows
        for (int j = 0; j < image_data[i].size(); ++j) { // Loop over elements in the row
            data_point = { i, j };
            gps = pixelToGPS(data_point, image_width, image_height, mapBounds.topLeft, mapBounds.topRight, mapBounds.bottomLeft, mapBounds.bottomRight);
            image_data[i][j].value = interpolate(input_data, gps.longitude, gps.latitude);
        }
    }
    std::cout << " hehehe" << image_data.size() << std::endl;
    wxImage::AddHandler(new wxPNGHandler);  // Add PNG image handler

    MyFrame* frame = new MyFrame(image_data);
    frame->SetClientSize(800, 600);
    frame->Show(true);
    return true;
}

MyFrame::MyFrame(const std::vector<std::vector<pixel_data>>& data)
    : wxFrame(NULL, wxID_ANY, "Michael's Commute Optimiser"),
    image_data(data)
{
    // Initialize mapBounds in the constructor
    
    // Set the background style to wxBG_STYLE_PAINT
    SetBackgroundStyle(wxBG_STYLE_PAINT);
    // Load the background image (change path to your image file)
    m_backgroundImage.LoadFile("C:/Users/michael.h_chemwatch/source/repos/commute/map.png", wxBITMAP_TYPE_PNG);

    Bind(wxEVT_PAINT, &MyFrame::OnPaint, this);
    Bind(wxEVT_MOUSEWHEEL, &MyFrame::OnMouseWheel, this);
    Bind(wxEVT_LEFT_DOWN, &MyFrame::OnLeftMouseDown, this);
    Bind(wxEVT_RIGHT_DOWN, &MyFrame::OnRightMouseDown, this);
    Bind(wxEVT_MOTION, &MyFrame::OnMouseMove, this);
    Bind(wxEVT_LEFT_UP, &MyFrame::OnMouseUp, this);

    wxMenu* menuFile = new wxMenu;
    menuFile->Append(ID_Hello, "&Hello...\tCtrl-H",
        "Help string shown in status bar for this menu item");
    menuFile->AppendSeparator();
    menuFile->Append(wxID_EXIT);

    wxMenu* menuHelp = new wxMenu;
    menuHelp->Append(wxID_ABOUT);

    wxMenuBar* menuBar = new wxMenuBar;
    menuBar->Append(menuFile, "&File");
    menuBar->Append(menuHelp, "&Help");

    SetMenuBar(menuBar);

    CreateStatusBar();
    SetStatusText("Welcome to wxWidgets!");

    Bind(wxEVT_MENU, &MyFrame::OnHello, this, ID_Hello);
    Bind(wxEVT_MENU, &MyFrame::OnAbout, this, wxID_ABOUT);
    Bind(wxEVT_MENU, &MyFrame::OnExit, this, wxID_EXIT);
}

void MyFrame::OnPaint(wxPaintEvent& event)
{
    wxAutoBufferedPaintDC dc(this);

    // Clear the background with black color
    dc.SetBrush(*wxBLACK_BRUSH);
    dc.SetPen(*wxBLACK_PEN);
    dc.DrawRectangle(0, 0, GetClientSize().x, GetClientSize().y);  // Fill entire window with black

    if (m_backgroundImage.IsOk())
    {
        // Scale the image according to zoom level
        wxImage scaledImage = m_backgroundImage.ConvertToImage();
        scaledImage.Rescale(m_backgroundImage.GetWidth() * m_zoomLevel, m_backgroundImage.GetHeight() * m_zoomLevel);

        wxBitmap scaledBitmap(scaledImage);

        // Draw the scaled and panned image
        dc.DrawBitmap(scaledBitmap, m_panOffset.x, m_panOffset.y, false);
    }
    
    // Overlay heatmap on top of the background
    int imageWidth = m_backgroundImage.GetWidth();
    int imageHeight = m_backgroundImage.GetHeight();
    std::cout << image_data.size() << std::endl;
    std::cout << image_data[0].size() << std::endl;
    std::cout << imageHeight << " " << imageWidth << std::endl;
    for (int i = 0; i < imageHeight; ++i) {
        for (int j = 0; j < imageWidth; ++j) {


        double value = image_data[i][j].value;  // Get the value from the resized image_data
        wxColor color = wxColour(255 * (1.0 - value), 0, 255 * value); // Simple blue-red gradient

        // Adjust coordinates for zooming and panning
        int drawX = m_panOffset.x + j * m_zoomLevel;  // Scale by zoom level
        int drawY = m_panOffset.y + i * m_zoomLevel;  // Scale by zoom level

        // Make sure coordinates are within the window bounds
        if (drawX < GetClientSize().x && drawY < GetClientSize().y)
        {
            dc.SetBrush(wxBrush(color));
            dc.SetPen(wxPen(color));
            dc.DrawRectangle(drawX, drawY, m_zoomLevel, m_zoomLevel);  // Draw scaled rectangles for each heatmap point
        }
        }
        //std::cout << i << std::endl;
    }
    //std::cout << "finished_loop" << std::endl;
}

void MyFrame::OnMouseWheel(wxMouseEvent& event)
{
    int rotation = event.GetWheelRotation();
    double zoomFactor = 1.1;  // Change zoom factor here if needed

    if (rotation > 0)
        m_zoomLevel *= zoomFactor;
    else if (rotation < 0)
        m_zoomLevel /= zoomFactor;

    Refresh();  // Repaint to apply zoom
}

void MyFrame::OnMouseClick(wxMouseEvent& event)
{

    // Check if the background image is loaded
    if (!m_backgroundImage.IsOk())
    {
        wxLogError("Background image not loaded!");
        return;  // Return early if image is not loaded
    }

    // Get the mouse position relative to the window
    wxPoint mousePos = event.GetPosition();

    // Apply the pan offset to the mouse position (accounting for any panning)
    mousePos.x -= m_panOffset.x;
    mousePos.y -= m_panOffset.y;

    // Apply the zoom factor to adjust the coordinates based on the zoom level
    double scaledX = mousePos.x / m_zoomLevel;
    double scaledY = mousePos.y / m_zoomLevel;

}

void MyFrame::OnLeftMouseDown(wxMouseEvent& event)
{
    if (event.LeftIsDown())
    {
        // Start panning only if the left mouse button is clicked
        m_isPanning = true;
        m_lastMousePos = event.GetPosition();
        CaptureMouse();  // Capture mouse to track movement outside window bounds
    }

}


void MyFrame::OnRightMouseDown(wxMouseEvent& event)
{
    if (event.RightIsDown())
    {

        // Get the mouse position relative to the window
        wxPoint mousePos = event.GetPosition();

        // Apply the pan offset to the mouse position (accounting for any panning)
        mousePos.x -= m_panOffset.x;
        mousePos.y -= m_panOffset.y;

        // Apply the zoom factor to adjust the coordinates based on the zoom level
        double scaledX = mousePos.x / m_zoomLevel;
        double scaledY = mousePos.y / m_zoomLevel;

        // Output the coordinates
        PixelCoordinate MouseClick = { scaledX, scaledY };

        GPSCoordinate topLeft = mapBounds.topLeft;
        GPSCoordinate topRight = mapBounds.topRight;
        GPSCoordinate bottomLeft = mapBounds.bottomLeft;
        GPSCoordinate bottomRight = mapBounds.bottomRight;

        GPSCoordinate gps = pixelToGPS(MouseClick, 600, 500, topLeft, topRight, bottomLeft, bottomRight);

        // Output the coordinates
        wxLogMessage("Mouse clickexxxxd at: (%.2f, %.2f)", gps.latitude, gps.longitude);
    }

}

void MyFrame::OnMouseMove(wxMouseEvent& event)
{
    if (m_isPanning)
    {
        wxPoint currentMousePos = event.GetPosition();
        wxPoint delta = currentMousePos - m_lastMousePos;

        m_panOffset += delta;  // Update the pan offset by delta
        m_lastMousePos = currentMousePos;

        Refresh();  // Repaint to apply pan
    }
}

void MyFrame::OnMouseUp(wxMouseEvent& event)
{
    if (m_isPanning)
    {
        m_isPanning = false;
        ReleaseMouse();
    }
}

void MyFrame::OnExit(wxCommandEvent& event)
{
    Close(true);
}

void MyFrame::OnAbout(wxCommandEvent& event)
{
    wxMessageBox("This is a good tool",
        "About Commute Optimiser", wxOK | wxICON_INFORMATION);
}

void MyFrame::OnHello(wxCommandEvent& event)
{
    wxLogMessage("Hello!");
}




#endif //HEADER_H