#include <io/OVF_File.hpp>

#include <utility/Exception.hpp>

namespace IO
{
    OVF_File::OVF_File( std::string filename, VF_FileFormat format, const std::string comment ) : 
        filename(filename), format(format), comment(comment)
    {
        this->output_to_file = "";
        this->output_to_file.reserve( int( 0x08000000 ) );  // reserve 128[MByte]
        this->empty_line = "#\n";
        
        if ( this->format == VF_FileFormat::OVF_BIN8 ) 
            this->datatype = "Binary 8";
        else if ( this->format == VF_FileFormat::OVF_BIN4 )
            this->datatype = "Binary 4";
        else if( this->format == VF_FileFormat::OVF_TEXT )
            this->datatype = "Text";
    }

    void OVF_File::Write_Top_Header( const int n_segments )
    {
        this->n_segments = n_segments; 
        
        this->output_to_file += fmt::format( "# OOMMF OVF 2.0\n" );
        this->output_to_file += fmt::format( this->empty_line );
        
        this->output_to_file += fmt::format( "# Segment count: {}\n", this->n_segments );
        this->output_to_file += fmt::format( this->empty_line );
        
        Dump_to_File( this->output_to_file, this->filename );  // Dump to file
        this->output_to_file = "";  // reset output string buffer
    }
    
    void OVF_File::Write_Segment( const vectorfield& vf, const Data::Geometry& geometry)
    {
        this->output_to_file += fmt::format( "# Begin: Segment\n" );
        this->output_to_file += fmt::format( "# Begin: Header\n" );
        this->output_to_file += fmt::format( this->empty_line );
        
        this->output_to_file += fmt::format( "# Title: SPIRIT Version {}\n", Utility::version_full );
        this->output_to_file += fmt::format( this->empty_line );
        
        this->output_to_file += fmt::format( "# Desc: {}\n", this->comment );
        this->output_to_file += fmt::format( this->empty_line );
        
        // The value dimension is always 3 in this implementation since we are writting Vector3-data
        this->output_to_file += fmt::format( "# valuedim: {} ##Value dimension\n", 3 );
        this->output_to_file += fmt::format( "# valueunits: None None None\n" );
        this->output_to_file += fmt::format( "# valuelabels: spin_x_component spin_y_component "
                                       "spin_z_component \n" );
        this->output_to_file += fmt::format( this->empty_line );
        
        this->output_to_file += fmt::format( "## Fundamental mesh measurement unit. "
                                       "Treated as a label:\n" );
        this->output_to_file += fmt::format( "# meshunit: nm\n" );                  //// TODO: treat that
        this->output_to_file += fmt::format( this->empty_line );
        
        this->output_to_file += fmt::format( "# xmin: {}\n", geometry.bounds_min[0] );
        this->output_to_file += fmt::format( "# ymin: {}\n", geometry.bounds_min[1] );
        this->output_to_file += fmt::format( "# zmin: {}\n", geometry.bounds_min[2] );
        this->output_to_file += fmt::format( "# xmax: {}\n", geometry.bounds_max[0] );
        this->output_to_file += fmt::format( "# ymax: {}\n", geometry.bounds_max[1] );
        this->output_to_file += fmt::format( "# zmax: {}\n", geometry.bounds_max[2] );
        this->output_to_file += fmt::format( this->empty_line );
        
        // XXX: Spirit does not support irregular geometry yet. We are emmiting rectangular mesh
        this->output_to_file += fmt::format( "# meshtype: rectangular\n" );
        
        // XXX: maybe this is not true for every system
        this->output_to_file += fmt::format( "# xbase: {}\n", 0 );
        this->output_to_file += fmt::format( "# ybase: {}\n", 0 );
        this->output_to_file += fmt::format( "# zbase: {}\n", 0 );
        
        this->output_to_file += fmt::format( "# xstepsize: {}\n", 
                                       geometry.lattice_constant * geometry.bravais_vectors[0][0] );
        this->output_to_file += fmt::format( "# ystepsize: {}\n", 
                                       geometry.lattice_constant * geometry.bravais_vectors[1][1] );
        this->output_to_file += fmt::format( "# zstepsize: {}\n", 
                                       geometry.lattice_constant * geometry.bravais_vectors[2][2] );
        
        this->output_to_file += fmt::format( "# xnodes: {}\n", geometry.n_cells[0] );
        this->output_to_file += fmt::format( "# ynodes: {}\n", geometry.n_cells[1] );
        this->output_to_file += fmt::format( "# znodes: {}\n", geometry.n_cells[2] );
        this->output_to_file += fmt::format( this->empty_line );
        
        this->output_to_file += fmt::format( "# End: Header\n" );
        this->output_to_file += fmt::format( this->empty_line );
        
        Write_Segment_Data( vf, geometry );
    }

    void OVF_File::Write_Segment_Data( const vectorfield& vf, const Data::Geometry& geometry )
    {
        // Data
        this->output_to_file += fmt::format( "# Begin: Data {}\n", this->datatype );
        
        if ( format == VF_FileFormat::OVF_BIN8 || format == VF_FileFormat::OVF_BIN4 )
        {
            Append_String_to_File( this->output_to_file, this->filename );  // Append to the top header
            Write_Data_bin( vf );                                   // write the binary
            this->output_to_file = "\n";                            // new line will be printed after data
        }
        else if ( format == VF_FileFormat::OVF_TEXT )
        {
            Write_Data_txt( vf );
        }
        
        this->output_to_file += fmt::format( "# End: Data {}\n", this->datatype );
        
        this->output_to_file += fmt::format( "# End: Segment\n" );
        
        // after data writting append output if binary or dump output if text
        //if ( format == VF_FileFormat::OVF_BIN8 || format == VF_FileFormat::OVF_BIN4 )
            //Append_String_to_File( this->output_to_file, this->filename );
        //else if ( format == VF_FileFormat::OVF_TEXT )
            //Dump_to_File( this->output_to_file, this->filename );
        
        Append_String_to_File( this->output_to_file, this->filename );  // Append the #End keywords
        this->output_to_file = "";  // reset output string buffer
    }
    
    void OVF_File::Write_Data_bin( const vectorfield& vf )
    {
        // Open the file to append binary
        std::ofstream outputfile( this->filename, std::ios::out | std::ios::app | std::ios::binary ); 
        outputfile.seekp( std::ios::end );                      // go to the end of the header
        
        // float test value
        float ref_4b_test = *reinterpret_cast<float *>( &this->hex_4b_test );
        
        // double test value
        double ref_8b_test = *reinterpret_cast<double *>( &this->hex_8b_test );
        
        if( format == VF_FileFormat::OVF_BIN8 )
        {
            // Write binary test value
            outputfile.write( reinterpret_cast<char *>(&ref_8b_test), sizeof(ref_8b_test) );
            
            double buffer[3];
            	
            // Convert every vector of the vf into vector<double> and then write it out
            for( unsigned int i=0; i<vf.size(); ++i )
            {                    
                buffer[0] = static_cast<double>(vf[i][0]);
                buffer[1] = static_cast<double>(vf[i][1]);
                buffer[2] = static_cast<double>(vf[i][2]);
                
                outputfile.write( reinterpret_cast<char *>(&buffer[0]), 3 * sizeof(double) ); 
            }
        }
        else if( format == VF_FileFormat::OVF_BIN4 )
        {
            // write binary test value
            outputfile.write( reinterpret_cast<char *>(&ref_4b_test), sizeof(ref_4b_test) );
            
            float buffer[3];
            
            // convert every vector of the vf into vector<float> and then write it out
            for( unsigned int i=0; i<vf.size(); i++ )
            {
                buffer[0] = static_cast<float>(vf[i][0]);
                buffer[1] = static_cast<float>(vf[i][1]);
                buffer[2] = static_cast<float>(vf[i][2]);
                
                outputfile.write( reinterpret_cast<char *>( &buffer[0] ), 3 * sizeof(float) ); 
            }
        }
        
        outputfile.close();
    }

    void OVF_File::Write_Data_txt( const vectorfield& vf )
    {
        for (int iatom = 0; iatom < vf.size(); ++iatom)
        {
                this->output_to_file += fmt::format( "{:20.10f} {:20.10f} {:20.10f}\n", 
                                                      vf[iatom][0], vf[iatom][1], vf[iatom][2] );
        }
    }

    void OVF_File::write_image( const vectorfield& vf, const Data::Geometry& geometry )
    {
        Write_Top_Header(1);
        Write_Segment( vf, geometry );
    }

    void OVF_File::write_eigenmodes( const std::vector<std::shared_ptr<vectorfield>>& modes,
                                     const Data::Geometry& geometry )
    {
        Write_Top_Header( modes.size() );
        for (int i=0; i<modes.size(); i++)
        {
            if ( modes[i] != NULL )
                Write_Segment( *modes[i], geometry );
        }
    }

    void OVF_File::write_chain( const std::shared_ptr<Data::Spin_System_Chain>& chain )
    {
    }
}
