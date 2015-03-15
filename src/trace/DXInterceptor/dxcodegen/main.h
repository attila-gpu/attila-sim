////////////////////////////////////////////////////////////////////////////////
///
/// @mainpage
///
/// Aquesta petita utilitat genera automàticament codi C++ a partir de la
/// descripció d'un arxiu de configuració.
///
/// @section descr_sec Funcionament
///
/// Es tracta d'una eina per ajudar a implementar més ràpid el conjunt de
/// classes de la API Direct3D en la suite DXInterceptor.
///
/// A partir de la descripció d'un arxiu de configuració, el programa parseja
/// els arxius de capçalera C++ (.h/.hpp) en aquest indicats i genera
/// automàticament arxius de codi C++. És capaç d'extreure els tipus d'elements
/// del llenguatge C++ @c enum, @c struct i @c class, la descripció dels quals
/// emmagatzema en estructures de dades internes que utilitza per a generar el
/// codi.
///
/// El codi generat no és 100\% compilable i té moltes peculiaritats que el fan
/// inutilitzable fora de la suite DXInterceptor.
///
/// Més informació en @ref xml_config_page.
///
/// @section syntax_sec Sintaxi
///
/// <tt>dxcodegen [opcions] @ref xml_config_page "\<arxiu-de-configuracio\>"</tt>
///
/// @par Opcions:
///
/// @li --verbose [-V] \n Mostra informació ampliada del processament de l'arxiu de configuració i les seves seccions
/// @li --help [-H] \n Mostra l'ajuda de la eina
///
/// @note
/// Encara que aquesta eina s'ha programat <i>ad hoc</i> per acceptar els arxius
/// de capçalera de <i>Direct3D 9.0c</i> (d3d9.h, d3d9types.h i d3d9caps.h),
/// també podria funcionar sense massa canvis amb els arxius de les versions
/// <i>8.0</i>, <i>8.1</i> i la propera <i>10.0</i>.
///
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
///
/// @page xml_config_page Format de l'Arxiu de Configuració
///
/// L'arxiu de configuració permet indicar tota l'informació necessària per a
/// controlar tot el procés de generació automàtica de codi C++ del programa
/// DXCodeGenerator. Està definit amb el llenguatge XML.
///
/// Consta de dos apartats principals, el parser i el generador.
///
/// @see
/// Sobre el llenguatge XML: http://es.wikipedia.org/wiki/XML
///
/// @section xml_config_parser_page Parser
///
/// En aquest apartat definim quins arxius amb codi C++ haurà de processar el
/// programa i quins dels elements del llenguatge C++, dintre d'aquests, haurà
/// d'extreure i emmagatzemar en estructures de dades internes per aplicar la
/// generació automàtica de codi C++ en la @ref xml_config_generator_page
/// "segona part" de l'execució del
/// programa.
///
/// Consta de les cinc seccions següents:
///
/// @subsection xml_config_parser_files_subsec Arxius
///
/// Indica al programa la llista d'arxius a parsejar. Disposa d'un atribut
/// anomenat @c basepath que serà on el programa cercarà els arxius indicats en
/// les subseccions @c files.
///
/// Si l'atribut @c basepath no existeix, està buit o el directori no existeix,
/// el programa suposarà el directori per defecte d'execució del programa. Si
/// algun dels arxius no existeix, el programa mostrarà un error per pantalla i
/// no continuarà.
///
/// @par Exemple:
///
/// @code
/// <?xml version="1.0" encoding="utf-8" standalone="yes" ?>
/// <configuration>
///   <parser>
///     <files basepath="c:/dx8sdk_headers/">
///     <file>d3d8.h</file>
///     <file>d3d8caps.h</file>
///     <file>d3d8types.h</file>
///     </files>
///   </parser>
/// </configuration>
/// @endcode
///
/// En aquest exemple li estem indicant al programa que cerqui els arxius en la
/// carpeta <tt>c:\\dx8sdk_headers\\</tt> i parsegi els arxius @c d3d8.h,
/// @c d3d8caps.h i @c d3d8types.h que estan en el seu interior.
///
/// @subsection xml_config_parser_enums_subsec Enumeracions, Estructures i Classes
///
/// Aquests apartats indiquen al programa quins elements del llenguatge C++ ha
/// de cercar en els arxius d'entrada. Corresponen respectivament a @c enum,
/// @c struct i @c class.
///
/// Tots tres tenen la mateixa sintaxi i consta de dos apartats: @c include i
/// @c exclude. Per defecte el programa intenta cercar tots els elements
/// possibles de cada tipus, però amb l'ajuda d'aquests apartats podem filtrar i
/// seleccionar només aquells elements que ens interessen per a la generació
/// automàtica de codi.
///
/// L'apartat @c include és el filtre més restrictiu i el programa només cercarà
/// els elements que estiguin dintre d'aquesta llista. En canvi l'apartat
/// @c exclude indica la llista d'elements que no volem parsejar d'aquell tipus,
/// deixant al programa que tracti tota la resta. Si un element es troba al
/// mateix temps en les seccions @c include i @c exclude té preferència la
/// secció @c exclude i per tant quedarà exclòs.
///
/// @par Exemple:
///
/// Donat el següent arxiu d'entrada, que anomenarem @c test.h :
///
/// @code
/// enum Flags { FLAG1, FLAG2 = 1000 };
///
/// enum Types
/// {
///   OPEN  = 0x00000001,
///   CLOSE = 0x00000002,
///   READ  = 0x00000004,
///   WRITE = 0x00000008,
/// };
///
/// struct Params
/// {
///   int   Field1;
///   long  Field2;
///   Types ParamsType;
/// };
///
/// struct Options
/// {
///   int   Option1;
///   long  Option2;
///   Flags OptionsFlags;
/// };
///
/// class A
/// {
/// public:
///   A(int code);
///   int GetCode();
/// protected:
///   int m_code;
/// };
///
/// class B
/// {
/// public:
///   B(std::string name);
///   std::string& GetName();
/// protected:
///   std::string m_name;
/// };
/// @endcode
///
/// I el següent arxiu de configuració:
///
/// @code
/// <?xml version="1.0" encoding="utf-8" standalone="yes" ?>
/// <configuration>
///   <parser>
///     <files>
///       <file>test.h</file>
///     </files>
///     <enums>
///       <exclude>Flags</exclude>
///     </enums>
///     <structs>
///      <include>Params</include>
///     </structs>
///     <classes>
///     </classes>
///   </parser>
/// </configuration>
/// @endcode
///
/// El programa extrauria els elements:
///
/// @li Només <tt>enum Types</tt> perquè hem exclòs @c Flags
/// @li Només <tt>struct Params</tt> perquè és l'únic element en la llista @c include
/// @li Les classes @c A i @c B, ja que no hem aplicat cap filtre per als elements d'aquest tipus
///
/// @subsection xml_config_parser_macros_subsec Macros
///
/// Permet al programa aplicar algunes transformacions senzilles sobre el codi
/// amb una sintaxi semblant a la directiva @c \#define de C/C++. Amb aquesta
/// eina podem evitar, en la majoria dels casos, haver de modificar a mà els
/// arxius d'entrada per poder-los parsejar més fàcilment pel programa. També es
/// fa servir per substituir algunes macros de C++ en els arxius d'entrada, tal
/// i com ho faria un compilador de C++ en la fase del preprocessador.
///
/// Cada secció consta de dos parts: @c left i @c right. Equivalen, més o menys,
/// a la part esquerra i dreta de la directiva @c \#define de C++.
///
/// @par Exemple:
///
/// Donat el següent arxiu d'entrada, que anomenarem @c test.h :
///
/// @code
/// #define DECLARE_INTERFACE_(iface,baseiface) class iface : public baseiface
/// #define STDMETHOD_(type,method) type method
/// #define PURE =0
///
/// DECLARE_INTERFACE_(A, A_Base)
/// {
/// public:
///   A(int code);
///   STDMETHOD_(int,GetCode)();
///   void SetCode() PURE;
/// protected:
///   int m_code;
/// };
/// @endcode
///
/// @note
/// Fixem-nos que el programa <b>NO</b> parseja les diretives @c \#define dels
/// arxius d'entrada, per tant el programa no veurà les primeres línies de
/// l'arxiu anterior. El programa només veu els elements @c enum, @c struct i
/// @c class.
///
/// I el següent arxiu de configuració:
///
/// @code
/// <?xml version="1.0" encoding="utf-8" standalone="yes" ?>
/// <configuration>
///   <parser>
///     <files>
///       <file>test.h</file>
///     </files>
///     <macros>
///       <macro>
///         <left>DECLARE_INTERFACE_(iface, baseiface)</left>
///         <right>class iface : public baseiface</right>
///       </macro>
///       <macro>
///         <left>STDMETHOD_(type,method)</left>
///         <right>type method</right>
///       </macro>
///       <macro>
///         <left>PURE</left>
///         <right></right>
///       </macro>
///     </macros>
///   </parser>
/// </configuration>
/// @endcode
///
/// Abans d'iniciar l'extracció dels elements el programa veuria:
///
/// @code
/// class A : public A_Base
/// {
/// public:
///   A(int code);
///   int GetCode();
///   void SetCode();
/// protected:
///   int m_code;
/// };
/// @endcode
///
/// On les macros s'haurien aplicat de la manera següent:
///
/// @li @c DECLARE_INTERFACE_ ha substituït la definició de la classe per <tt>class A : public A_Base</tt>
/// @li @c STDMETHOD_ ha substituït la definició del mètode @c GetCode per <tt>int GetCode();</tt>
/// @li @c PURE s'ha substituït per la cadena buida i per tant s'ha eliminat de darrera la definició del mètode @c SetCode
///
/// @section xml_config_generator_page Generador
///
/// En aquest apartat definim com es realitzarà la generació automàtica de codi
/// codi C++ a partir de les dades obtingudes en l'execució de l'apartat
/// anterior.
///
/// El codi generat no és 100% compilable i té moltes peculiaritats que el fan
/// inutilitzable fora de la suite DXInterceptor. Totes les classes estan
/// generades a mida per a la resta de projectes de la suite que les fan servir.
///
/// El programa genera tres classes ajudants per cadascun del tipus d'elements
/// de C++ que el programa detecta i extreu.
///
/// @par DXEnumHelper
///
/// Classe ajudant que disposa de mètodes per a transformar valors binaris en
/// cadenes de text i viceversa per cadascun dels elements de tipus @c enum que
/// el programa hagi trobat en la fase de parsing.
///
/// Per cada @c enum el programa generarà dos mètodes:
///
/// @li <b>\<nom-enum\>_ToString</b>: Donat un valor del tipus del @c enum en retorna una cadena de text
/// @li <b>\<nom-enum\>_FromString</b>: Donada una cadena de text ens retorna el valor binari del element del @c enum
///
/// Per exemple, donat el @c enum següent:
///
/// @code
/// enum ACTIONTYPES
/// {
///   OPEN  = 0x00000001,
///   CLOSE = 0x00000002,
///   READ  = 0x00000004,
///   WRITE = 0x00000008,
/// };
/// @endcode
///
/// El programa generarà els mètodes:
///
/// @code
/// const std::string& ACTIONTYPES_ToString(ACTIONTYPES value);
/// ACTIONTYPES ACTIONTYPES_FromString(const std::string& value);
/// @endcode
///
/// Que utilitzarem així:
///
/// @code
/// ACTIONTYPES_ToString(0x02) --> "CLOSE"
/// ACTIONTYPES_FromString("WRITE") --> 0x08
/// @endcode
///
/// @par DXStructHelper
///
/// Classe ajudant que disposa de mètodes per a transformar en cadenes de text
/// cadascun dels elements de tipus @c struct que el programa hagi trobat en la
/// fase de parsing.
///
/// Per cada @c struct el programa generarà un mètode <b>\<nom-struct\>_ToString</b>
/// que donada una variable d'aquest tipus transformarà en una cadena de text.
///
/// @par DXMethodCallHelper
///
///
////////////////////////////////////////////////////////////////////////////////
