##############################################################
#
# Wrapper C++ implementation header/source file generation
#

package Wrapper;

use File::Basename;

use strict;
use Utils;
use Parser;
use Dispatch;

our $common_inc = "<common.h>";

##############################################################
# Header file generation

sub genHdrPreamble($$) {
  my ($cls, $qifname) = @_;

  my $cpp_decl_hdrname = qif2CliHdrFname($qifname);

  my $hdr_defined_macro = "MCWG_${qifname}_WRAPPER_HPP__";
  print OUT "//";
  print OUT "// Auto-generated by mcwrapgen3.pl. Don't edit.";
  print OUT "//";
  print OUT "\n";
  print OUT "#ifndef $hdr_defined_macro\n";
  print OUT "#define $hdr_defined_macro\n";
  print OUT "\n";

  print OUT "#include <qlib/LClassUtils.hpp>\n";
  print OUT "#include <qlib/LWrapper.hpp>\n";
  print OUT "#include <qlib/SingletonBase.hpp>\n";
  print OUT "\n";

  if ($cpp_decl_hdrname) {
    print OUT "#include \"$cpp_decl_hdrname\"\n";
    print OUT "\n";
  }
  else {
      print STDERR "Warning ($qifname): C++ class decl header file is not defined.";
  }
}

sub genHdrClassDecl($$$$$) {
  my ($cls, $qifname, $cpp_decl_hdrname, $cpp_cli_clsname, $cpp_wp_clsname) = @_;

  my $cls = shift;
  my $ropts = $cls->{"options"};
  my $clscls_super;

  if (contains($ropts, "singleton")) {
      $clscls_super = "qlib::LSingletonSpecificClass<$cpp_cli_clsname>";
  }
  elsif (contains($ropts, "smartptr")) {
      $clscls_super = "qlib::LSmartPtrSupportClass<$cpp_cli_clsname>";
  }
  else {
      $clscls_super = "qlib::LSpecificClass<$cpp_cli_clsname>";
  }

  print OUT "//\n";
  print OUT "// Wrapper class for $cpp_cli_clsname\n";
  print OUT "//\n";
  print OUT "\n";
  print OUT "class $cpp_wp_clsname :\n";
  print OUT "  public $clscls_super,\n";
  print OUT "  public qlib::SingletonBase<$cpp_wp_clsname>,\n";
  print OUT "  public qlib::LWrapperImpl\n";
  print OUT "{\n";

  # Declare typedefs
  print OUT "public:\n";
  print OUT "  typedef $cpp_cli_clsname client_t;\n";
  print OUT "  typedef $clscls_super super_t;\n";
  print OUT "\n";

  # Declare ctor/dtor
  print OUT "\n";
  print OUT "public:\n";
  print OUT "  ${cpp_wp_clsname}() : super_t(\"$qifname\")\n";
  print OUT "  {\n";
  print OUT "    funcReg(this);\n";
  print OUT "  }\n";
  print OUT "\n";
  print OUT "  virtual ~${cpp_wp_clsname}()\n";
  print OUT "  {\n";
  print OUT "    // MB_DPRINTLN(\"$cpp_wp_clsname destructed\");\n";
  print OUT "  }\n";
  print OUT "\n";

  # Funcmap registration
  print OUT "  // setup function map (called by init())\n";
  print OUT "  static void funcReg(qlib::FuncMap *pmap);\n";
  print OUT "\n";

  # Dispatch invocation decl
  print OUT "\n";
  print OUT "  //\n";
  print OUT "  // Dispatch interfaces\n";
  print OUT "  //\n";
  Dispatch::genPropDecl($cls);
  Dispatch::genInvokeDecl($cls);

  # print OUT "\n";
  # print OUT "  //\n";
  # print OUT "  virtual qlib::LString getScrClassName() const { return getClassName(); }\n";

  print OUT "\n";
  print OUT "};\n";

  my $modifier = $cls->{"dllexport"};
  print OUT "\n";
  print OUT "$modifier void ${cpp_wp_clsname}_funcReg(qlib::FuncMap *pmap);\n";
}

##############################################################
# Source C++ file generation

sub genSrcPreamble($$$) {
  my $fhout = shift;
  my $qif = shift;
  my $wrap_header = shift; #qif2WpHdrFname($qif);

  my $cls;
  if ($qif) {
    $cls = $Parser::db{$qif};
  }

  print $fhout "//\n";
  print $fhout "// Auto-generated by mcwrapgen3.pl. Don't edit.\n";
  print $fhout "//\n";
  print $fhout "\n";
  print $fhout "#include $common_inc\n";

  if ($wrap_header) {
    my ($in_base, $in_dir, $in_ext) = fileparse($wrap_header, '\.hpp');
    print $fhout "\#include \"$in_base$in_ext\"\n";
  }
  print $fhout "\n";
  print $fhout "\#include <qlib/ClassRegistry.hpp>\n";
  print $fhout "\#include <qlib/LVariant.hpp>\n";
  # print $fhout "\#include <qlib/LPropEvent.hpp>\n";
  print $fhout "\n";

  foreach my $f (@Parser::user_cxx_incfiles) {
    print $fhout "\#include $f\n";
  }

  if ($cls && $cls->{"refers"}) {
      foreach my $icls (@{$cls->{"refers"}}) {
	  # debug "***** REFERS $icls";
	  my $hdrname = qif2WpHdrFname($icls);
	  print $fhout "#include \"$hdrname\"\n";
      }
      print $fhout "\n";
  }

  print $fhout "\n";
  print $fhout "using qlib::LString;\n";
  print $fhout "using qlib::LVariant;\n";
  print $fhout "using qlib::LVarArgs;\n";
  print $fhout "using qlib::LClass;\n";
  print $fhout "using qlib::ClassRegistry;\n";
  print $fhout "\n";
}
  
sub genSrcClsLdr($) {
  my $cls = shift;
  my $ropts = $cls->{"options"};

  my $qifname = $cls->{"qifname"};
  my $cpp_wp_clsname = qif2WpClsName($qifname);
  my $cpp_cli_clsname = qif2CliClsName($qifname);

  my $cpp_clsobj_name = "sClassObj_$qifname";
  my $cpp_clscls_name = "LSpecificClass_$qifname";

  # property/invocation code
  print OUT "\n";

  print OUT "SINGLETON_BASE_IMPL($cpp_wp_clsname);\n";
  print OUT "\n";

  if (contains($ropts, "cloneable")) {
    print OUT "MC_CLONEABLE_IMPL($cpp_cli_clsname);\n";
    print OUT "\n";
  }

  print OUT "MC_DYNCLASS_IMPL2($cpp_cli_clsname, $cpp_wp_clsname);\n";
  print OUT "MC_PROP_IMPL2($cpp_cli_clsname, $cpp_wp_clsname);\n";
  print OUT "MC_INVOKE_IMPL2($cpp_cli_clsname, $cpp_wp_clsname);\n";
  print OUT "\n";
}

################################

sub genCxxHeader($) {

  my $cls = shift;

  my $qifname = $cls->{"qifname"};

  my $wp_hdr_fname = qif2WpHdrFname($qifname);
  my $cpp_decl_hdrname = qif2CliHdrFname($qifname);
  my $cpp_cli_clsname = qif2CliClsName($qifname);
  my $cpp_wp_clsname = qif2WpClsName($qifname);

  debug("qif: $qifname\n");
  debug("$cpp_cli_clsname\n");
  debug("$cpp_wp_clsname\n");

  if (!$cpp_cli_clsname || !$cpp_wp_clsname) {
    die("ERROR: C++ class name (cpp_name) is required in input file");
  }

  print("Output C++ header file: $wp_hdr_fname\n");

  open(OUT, ">$wp_hdr_fname") || die "$?:$!";
  set_building_file($wp_hdr_fname);
  $Dispatch::fhout = *OUT;

  genHdrPreamble($cls, $qifname);

  genHdrClassDecl($cls,
                  $qifname,
                  $cpp_decl_hdrname,
                  $cpp_cli_clsname,
                  $cpp_wp_clsname);

  ###

  print OUT "\n";
  print OUT "#endif\n";
  print OUT "\n";

  close(OUT);
}

sub genCxxSource($) {
  my $cls = shift;

  my $qifname = $cls->{"qifname"};
  my $out_fname = qif2WpSrcFname($qifname);
  # my $cpp_decl_hdrname = qif2CliHdrFname($qifname);
  my $cpp_cli_clsname = qif2CliClsName($qifname);
  my $cpp_wp_clsname = qif2WpClsName($qifname);

  if (!$cpp_cli_clsname || !$cpp_wp_clsname) {
    die("ERROR: C++ class name (cpp_name) is required in input file");
  }

  print("Output C++ source file: $out_fname\n");

  open(OUT, ">$out_fname") || die "$?:$!";
  set_building_file($out_fname);
  $Dispatch::fhout = *OUT;

  if (contains($cls->{"options"}, "smartptr")) {
    $cpp_cli_clsname = "qlib::LScrSp<$cpp_cli_clsname>";
  }

  genSrcPreamble(*OUT, $qifname, qif2WpHdrFname($qifname));

  print OUT "/////////////////////////////////////\n";
  print OUT "// Class loader code for the client class $cpp_cli_clsname\n";
  genSrcClsLdr($cls);
  print OUT "\n";

  print OUT "/////////////////////////////////////\n";
  print OUT "//\n";
  print OUT "// Wrapper class for $cpp_cli_clsname\n";
  print OUT "//\n";
  print OUT "\n";

  print OUT "/////////////////////////////////////\n";
  print OUT "// Dispatch interface code\n";
  print OUT "\n";
  print OUT "//\n";
  print OUT "// Property getter/setter wrappers\n";
  print OUT "//\n";
  Dispatch::genPropertyCode($cls);

  print OUT "\n";
  print OUT "//\n";
  print OUT "// Method invocation wrappers\n";
  print OUT "//\n";
  Dispatch::genInvokeCode($cls);

  print OUT "\n";
  print OUT "//\n";
  print OUT "// Function table registration code\n";
  print OUT "//\n";
  Dispatch::genRegFuncCode($cls);


  close(OUT);
}
		   
1;

